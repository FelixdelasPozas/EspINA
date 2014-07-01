/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// EspINA
#include "TemporalStorage.h"

// C++
#include <iostream>

// Qt
#include <QStack>
#include <QDebug>

namespace EspINA
{
  QList<TemporalStorage *> TemporalStorage::s_Storages;

  //----------------------------------------------------------------------------
  bool removeRecursively(const QString & dirName)
  {
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName))
    {
      for(QFileInfo info : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (info.isDir())
        {
          result = removeRecursively(info.absoluteFilePath());
        }
        else
        {
          result = QFile::remove(info.absoluteFilePath());
        }

        if (!result)
        {
          return result;
        }
      }
      result = dir.rmdir(dirName);
    }

    return result;
  }

  //----------------------------------------------------------------------------
  TemporalStorage::TemporalStorage(const QDir* parent)
  {
    QDir tmpDir;

    if (parent)
    {
      tmpDir = *parent;
    }
    else
    {
      tmpDir = QDir::tempPath();
      tmpDir.mkpath("espina");
      tmpDir.cd("espina");
    }

    QString path = QUuid::createUuid().toString();

    tmpDir.mkdir(path);

    m_storageDir = QDir(tmpDir.absoluteFilePath(path));

    s_Storages << this;
  }

  //----------------------------------------------------------------------------
  TemporalStorage::~TemporalStorage()
  {
    removeRecursively(m_storageDir.absolutePath());
    s_Storages.removeOne(this);
  }

  //----------------------------------------------------------------------------
  QString TemporalStorage::findFile(const QString &fileName) const
  {
    for(auto storage: s_Storages)
    {
      QStack<QString> stack;
      stack.push(storage->m_storageDir.absolutePath());

      while (!stack.isEmpty())
      {
         QString sSubdir = stack.pop();
         QDir subdir(sSubdir);

         // Check for the file
         QStringList entries = subdir.entryList(QStringList() << fileName, QDir::Files);
         if (!entries.empty())
           return storage->m_storageDir.absoluteFilePath(fileName);

         QFileInfoList infoEntries = subdir.entryInfoList(QStringList(),
                                                          QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
         for (int i = 0; i < infoEntries.size(); i++) {
            QFileInfo& item = infoEntries[i];
            stack.push(item.absoluteFilePath());
         }
      }
    }

    return QString();
  }

  //----------------------------------------------------------------------------
  void TemporalStorage::saveSnapshot(SnapshotData data)
  {
    QFileInfo fileName(m_storageDir.absoluteFilePath(data.first));

    m_storageDir.mkpath(fileName.absolutePath());

    QFile file(fileName.absoluteFilePath());
    file.open(QIODevice::WriteOnly);
    file.write(data.second);
    file.close();
  }

  //----------------------------------------------------------------------------
  QByteArray TemporalStorage::snapshot(const QString& descriptor) const
  {
    QString fileName = m_storageDir.absoluteFilePath(descriptor);

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    return data;
  }

  //----------------------------------------------------------------------------
  Snapshot TemporalStorage::snapshots(const QString& relativePath, TemporalStorage::Mode mode) const
  {
    Snapshot result;

    QDir dir = m_storageDir.absoluteFilePath(relativePath);

    for (auto file : dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot))
    {
      QString relativeStoragePath = m_storageDir.relativeFilePath(file.absoluteFilePath());
      if (file.isDir())
      {
        if (Mode::Recursive == mode)
        {
          result << snapshots(relativeStoragePath, mode);
        }
      }
      else
      {
        result << SnapshotData(relativeStoragePath, snapshot(relativeStoragePath));
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------
  bool TemporalStorage::exists(const QString &name)
  {
    QFileInfo file(m_storageDir.absoluteFilePath(name));
    return file.exists();
  }

  //----------------------------------------------------------------------------
  bool removeTemporalDirectory()
  {
    QDir temporalPath = QDir::tempPath();
    if (!temporalPath.cd("espina"))
      return false;

    return removeRecursively(temporalPath.absolutePath());
  }

} // namespace EspINA
