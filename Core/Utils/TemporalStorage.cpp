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

// ESPINA
#include "TemporalStorage.h"
#include "EspinaException.h"

// C++
#include <unistd.h>
#include <iostream>

// Qt
#include <QStack>
#include <QObject>
#include <QDebug>
#include <QtGui>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

QList<TemporalStorage *> TemporalStorage::s_Storages;
const QString SETTINGS_FILE = "Settings/SessionSettings.ini";

// C++
#ifdef __WIN64__
#include <fileapi.h>
#endif

//---------------------------------------------------------------------------
std::string ESPINA::getShortFileName(const QString &filename)
{
  // NOTE 1: this seems to work if you pass this to the ITK filters. See note 2.
  return QDir::fromNativeSeparators(filename).toLatin1().toStdString();

  // NOTE 2: Opening files in Windows (with UTF-8 names) can be difficult with
  // ITK methods that use "char *" for file names. The following code gets the
  // 8.3 name of the given UTF-8 file name.
#ifdef __WIN64__
  long     length = 0;
  TCHAR*   buffer = nullptr;
  const auto name = QDir::fromNativeSeparators(filename).toLatin1().data();

  // First obtain the size needed by passing NULL and 0.
  length = GetShortPathNameA(name, nullptr, 0);
  if (length == 0)
  {
    return filename.toLatin1().toStdString();
  }

  // Dynamically allocate the correct size
  // (terminating null char was included in length)
  buffer = new TCHAR[length];

  // Now simply call again using same long path.
  length = GetShortPathNameA(name, buffer, length);
  if (length == 0)
  {
    delete [] buffer;

    return filename.toLatin1().toStdString();
  }

  const auto result = std::string(buffer, length);

  delete [] buffer;
  return result;
#else
  return filename.toStdString();
#endif
}

//----------------------------------------------------------------------------
bool moveRecursively(const QString &sourceDir, const QString &destinationDir, bool createDestination = false)
{
  bool result = false;
  QFileInfo source(sourceDir);
  QFileInfo destination(destinationDir);

  if(!source.isDir() || !destination.isDir())
  {
    return false;
  }

  if(!destination.exists())
  {
    if(createDestination)
    {
      QDir to(destinationDir);
      to.mkdir(".");
    }
    else
    {
      return false;
    }
  }

  if(source.exists() && source.isReadable())
  {
    result = true;
    QDir from(sourceDir);
    QDir to(destinationDir);

    for (QFileInfo info: from.entryInfoList(QDir::NoDotAndDotDot|QDir::System|QDir::Hidden|QDir::AllDirs|QDir::Files, QDir::DirsFirst))
    {
      if (info.isDir())
      {
        auto subDir = to;
        if (!subDir.mkdir(info.baseName()) || !subDir.cd(info.baseName()))
        {
          return false;
        }

        result &= moveRecursively(info.absoluteFilePath(), subDir.absolutePath());
      }
      else
      {
        result &= QFile::copy(info.absoluteFilePath(), to.absoluteFilePath(info.baseName()));
        result &= QFile::remove(info.absoluteFilePath());
      }

      if (!result) return result;
    }
  }

  return result;
}

//----------------------------------------------------------------------------
bool removeRecursively(const QString &dirName)
{
  bool result = true;
  QDir dir(dirName);

  if (dir.exists(dirName))
  {
    for (QFileInfo info : dir.entryInfoList(QDir::NoDotAndDotDot|QDir::System|QDir::Hidden|QDir::AllDirs|QDir::Files, QDir::DirsFirst))
    {
      if (info.isDir())
      {
        result &= removeRecursively(info.absoluteFilePath());
      }
      else
      {
        result &= QFile::remove(info.absoluteFilePath());
      }

      if (!result) return result;
    }

    result &= dir.rmdir(dirName);
  }

  return result;
}

//----------------------------------------------------------------------------
TemporalStorage::TemporalStorage(const QDir *parent)
: m_uuid    {QUuid::createUuid()}
, m_settings{nullptr}
{
  if (parent)
  {
    m_baseStorageDir = *parent;
  }
  else
  {
    m_baseStorageDir = QDir::tempPath();
  }

  auto tmpDir = m_baseStorageDir;

  if(!tmpDir.mkpath("espina") || !tmpDir.cd("espina"))
  {
    auto message = QObject::tr("Can't create 'espina' path in: %1").arg(tmpDir.absolutePath());
    auto details = QObject::tr("TemporalStorage::TemporalStorage() -> ") + message;

    throw EspinaException(message, details);
  }

  QString path = m_uuid.toString();

  if (!tmpDir.mkdir(path))
  {
    auto message = QObject::tr("Can't create main storage path: %1").arg(path);
    auto details = QObject::tr("TemporalStorage::TemporalStorage() -> ") + message;

    throw EspinaException(message, details);
  }

  m_storageDir = QDir{tmpDir.absoluteFilePath(path)};

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
  for (auto storage : s_Storages)
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
      {
        return QDir::fromNativeSeparators(storage->m_storageDir.absoluteFilePath(fileName));
      }

      QFileInfoList infoEntries = subdir.entryInfoList(QStringList(), QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
      for (int i = 0; i < infoEntries.size(); i++)
      {
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

  if(!m_storageDir.mkpath(fileName.absolutePath()))
  {
    auto message = QObject::tr("Can't create path: %1").arg(fileName.absolutePath());
    auto details = QObject::tr("TemporalStorage::saveSnapshot() -> ") + message;

    throw EspinaException(message, details);
  }

  QFile file(fileName.absoluteFilePath());
  if (!file.open(QIODevice::WriteOnly))
  {
    auto message = QObject::tr("Can't create file: %1").arg(fileName.absoluteFilePath()).arg(file.errorString());
    auto details = QObject::tr("TemporalStorage::saveSnapshot() -> ") + message;

    throw EspinaException(message, details);
  }
  else
  {
    if(-1 == file.write(data.second))
    {
      auto message = QObject::tr("Can't write data to file: %1. Error: %2").arg(fileName.absoluteFilePath()).arg(file.errorString());
      auto details = QObject::tr("TemporalStorage::saveSnapshot() -> ") + message;

      throw EspinaException(message, details);
    }

    if(!file.flush() || file.error() != QFileDevice::NoError)
    {
      auto message = QObject::tr("Error flushing or writing file: %1. Error: %2").arg(fileName.absoluteFilePath()).arg(file.errorString());
      auto details = QObject::tr("TemporalStorage::saveSnapshot() -> ") + message;

      throw EspinaException(message, details);
    }

    file.close();
  }
}

//----------------------------------------------------------------------------
QByteArray TemporalStorage::snapshot(const QString &descriptor) const
{
  QString fileName = QDir::fromNativeSeparators(m_storageDir.absoluteFilePath(descriptor));

  QByteArray data;

  QFile file(fileName);
  if (file.exists() && file.open(QIODevice::ReadOnly))
  {
    data = file.readAll();
    file.close();
  }
  else
  {
    qWarning() << "TemporalStorage::snapshot() -> can't open:" << fileName;
    qWarning() << "TemporalStorage::snapshot() -> Cause:" << file.errorString();
  }

  return data;
}

//----------------------------------------------------------------------------
Snapshot TemporalStorage::snapshots(const QString &relativePath, TemporalStorage::Mode mode) const
{
  Snapshot result;

  QDir dir = QDir::fromNativeSeparators(m_storageDir.absoluteFilePath(relativePath));

  for (auto file : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot))
  {
    QString relativeStoragePath = m_storageDir.relativeFilePath(file.absoluteFilePath());
    if (file.isDir())
    {
      if (Mode::RECURSIVE == mode)
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
bool TemporalStorage::exists(const QString &name) const
{
  QFileInfo file(m_storageDir.absoluteFilePath(QDir::fromNativeSeparators(name)));
  return file.exists();
}

//----------------------------------------------------------------------------
bool TemporalStorage::rename(const QString &oldName, const QString &newName) const
{
  QFileInfo oldFile(m_storageDir.absoluteFilePath(QDir::fromNativeSeparators(oldName)));
  QFileInfo newFile(m_storageDir.absoluteFilePath(QDir::fromNativeSeparators(newName)));

  return (oldFile.exists() && !newFile.exists() && QFile::rename(oldFile.absoluteFilePath(), newFile.absoluteFilePath()));
}

//----------------------------------------------------------------------------
bool TemporalStorage::move(const QString &path, bool createDir)
{
  if(QDir::fromNativeSeparators(path) == QDir::fromNativeSeparators(m_baseStorageDir.absolutePath()))
  {
    return true;
  }

  bool result = false;

  QDir newDir(path);
  if(!newDir.mkdir("espina") || !newDir.cd("espina") || !newDir.mkdir(m_uuid.toString()) || !newDir.cd(m_uuid.toString()))
  {
    auto message = QObject::tr("Can't create 'espina' path in: %1").arg(newDir.absolutePath());
    auto details = QObject::tr("TemporalStorage::move() -> ") + message;

    throw EspinaException(message, details);
  }
  else
  {
    result = moveRecursively(m_storageDir.absolutePath(), newDir.absolutePath(), createDir);
    if(result)
    {
      m_baseStorageDir = QDir(path);
      m_storageDir = QDir(newDir.absolutePath());
    }
  }

  return result;
}

//----------------------------------------------------------------------------
std::shared_ptr<QSettings> TemporalStorage::sessionSettings()
{
  return std::make_shared<QSettings>(QDir::fromNativeSeparators(m_storageDir.absoluteFilePath(SETTINGS_FILE)), QSettings::IniFormat);
}

//----------------------------------------------------------------------------
void TemporalStorage::makePath(const QString &path)
{
  if(!m_storageDir.mkpath(path))
  {
    auto message = QObject::tr("Couldn't create path '%1'.").arg(path);
    auto details = QObject::tr("TemporalStorage::makePath() -> ") + message;

    throw EspinaException(message, details);
  }
}

//----------------------------------------------------------------------------
void TemporalStorage::syncSessionSettings()
{
  auto settings = sessionSettings();
  settings->sync();
}

//----------------------------------------------------------------------------
bool ESPINA::removeTemporalDirectory(const QDir &path)
{
  QFileInfo info{path.absolutePath()};
  auto directory = path;

  if(!info.isDir() || !info.isWritable() || !directory.cd("espina"))
  {
    return false;
  }

  return removeRecursively(directory.absolutePath());
}
