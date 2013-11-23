/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "TemporalStorage.h"

#include <iostream>

using namespace std;
using namespace EspINA;

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
TemporalStorage::TemporalStorage(const QDir& parent)
{
  QString path = QUuid::createUuid().toString();

  parent.mkdir(path);

  m_storageDir = QDir(parent.absoluteFilePath(path));
}

//----------------------------------------------------------------------------
TemporalStorage::~TemporalStorage()
{
  removeRecursively(m_storageDir.absolutePath());
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