/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

// Qt
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QStack>
#include <QApplication>

//--------------------------------------------------------------------
void usage(const char *appName)
{
  qDebug() << "Usage: " << appName << " source_code_root_directory\n";
}

//--------------------------------------------------------------------
void error(const QString &message)
{
  qDebug() << "ERROR: " << message;
}

//--------------------------------------------------------------------
QStringList findFiles(QDir &dir)
{
  QStringList results;

  if(dir.exists())
  {
    dir.setFilter(QDir::NoDotAndDotDot|QDir::NoSymLinks|QDir::AllEntries);

    auto infoList = dir.entryInfoList();

    for(auto info: infoList)
    {
      if(info.absoluteFilePath().contains("/Test", Qt::CaseInsensitive)) continue;

      if(info.isFile() && info.fileName().endsWith(".h", Qt::CaseInsensitive))
      {
        results << info.absoluteFilePath();
      }

      if(info.isDir() && info.absoluteFilePath() != dir.absolutePath())
      {
        // app files don't need export.
        if(!info.absoluteFilePath().endsWith("app", Qt::CaseInsensitive))
        {
          auto nextDir = QDir{info.absoluteFilePath()};
          results << findFiles(nextDir);
        }
      }
    }
  }

  return results;
}

//--------------------------------------------------------------------
void processFile(const QString &filename)
{
  QFile file{filename};

  if(!file.open(QFile::ReadOnly))
  {
    error(QObject::tr("Can't open file '%1' for read only!").arg(filename));
  }

  QTextStream in(&file);
  int indent = 0;
  int lineCount = 0;
  QStack<int> classIndent;

  while (!in.atEnd())
  {
    auto line = in.readLine();
    ++lineCount;
    auto cleanLine = line.remove('\t').simplified();
    auto parts = cleanLine.split(" ");

    if(parts.size() == 0) continue;

    indent += cleanLine.count('{');

    auto dropIndent = cleanLine.count('}');
    if(dropIndent != 0 && !classIndent.isEmpty() && indent == classIndent.back())
    {
      classIndent.pop_back();
    }

    indent -= dropIndent;

    if(parts.at(0).startsWith("template", Qt::CaseInsensitive))
    {
      if(cleanLine.endsWith(";", Qt::CaseInsensitive)) continue;

      classIndent.push_back(indent + 1);
      continue;
    }

    if((parts.at(0).compare("class", Qt::CaseInsensitive) == 0) || (parts.at(0).compare("struct", Qt::CaseInsensitive) == 0))
    {
      classIndent.push_back(indent + 1);
      // skip forward declarations (with comments also).
      if((parts.size() >= 2) && (parts.at(1).endsWith(";", Qt::CaseInsensitive)))
      {
        classIndent.pop_back();
        continue;
      }

      if(parts.size() >= 2)
      {
        if(!parts.at(1).endsWith("export", Qt::CaseInsensitive))
        {
          qDebug() << "File:" << file.fileName() << "Line:" << lineCount << " - class" << parts.at(1) << "doesn't have Export definition.";
        }
      }
      continue;
    }

    auto beginP = cleanLine.indexOf('(');
    auto endP   = cleanLine.indexOf(')');
    if(cleanLine.endsWith(';') && beginP != -1 && endP != -1 && endP > beginP && classIndent.isEmpty() && !cleanLine.contains("EXPORT", Qt::CaseSensitive))
    {
      qDebug() << "File:" << file.fileName() << "Line:" << lineCount << "Possible method: " << cleanLine;
    }
  }

  file.close();
}

//--------------------------------------------------------------------
int main(int argc, char **argv)
{
  if(argc != 2 )
  {
    usage(argv[0]);
    return 1;
  }

  auto rootDir = QDir{argv[1]};
  qDebug() << "start directory:" << rootDir.absolutePath();

  if(!rootDir.exists())
  {
    error(QObject::tr("The directory '%1' does not exists!").arg(argv[1]));
    return 1;
  }

  auto files = findFiles(rootDir);

  qDebug() << "found" << files.size() << "files.";

  for(auto file: files)
  {
    processFile(file);
  }

  return 0;
}
