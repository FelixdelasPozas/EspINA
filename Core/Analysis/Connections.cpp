/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <Core/Analysis/Connections.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Utils/EspinaException.h>

// Qt
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

const QString ConnectionStorage::CONNECTIONS_STORAGE_FILE = "connections.bin";

//--------------------------------------------------------------------
bool ConnectionStorage::addConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3& point)
{
  QWriteLocker lock(&m_lock);

  auto uuid1  = segmentation1->uuid();
  auto uuid2  = segmentation2->uuid();
  auto vector = QVector3D{point[0], point[1], point[2]};

  if(m_data.keys().contains(uuid1) && m_data[uuid1].keys().contains(uuid2) && m_data[uuid1][uuid2].contains(vector))
  {
    return false;
  }

  m_data[uuid1][uuid2] << vector;
  m_data[uuid2][uuid1] << vector;

  return true;
}

//--------------------------------------------------------------------
bool ConnectionStorage::removeConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3& point)
{
  QWriteLocker lock(&m_lock);

  auto uuid1  = segmentation1->uuid();
  auto uuid2  = segmentation2->uuid();
  auto vector = QVector3D{point[0], point[1], point[2]};

  if(!m_data.keys().contains(uuid1) || !m_data[uuid1].keys().contains(uuid2) || !m_data[uuid1][uuid2].contains(vector))
  {
    return false;
  }

  m_data[uuid1][uuid2].removeAll(vector);
  m_data[uuid2][uuid1].removeAll(vector);

  checkAndRemoveIfEmtpy(uuid1, uuid2);
  checkAndRemoveIfEmtpy(uuid2, uuid1);

  return true;
}

//--------------------------------------------------------------------
bool ConnectionStorage::removeConnections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2)
{
  QWriteLocker lock(&m_lock);

  auto uuid1 = segmentation1->uuid();
  auto uuid2 = segmentation2->uuid();

  if(!m_data.keys().contains(uuid1) || !m_data[uuid1].keys().contains(uuid2))
  {
    return false;
  }

  m_data[uuid1].remove(uuid2);
  m_data[uuid2].remove(uuid1);

  checkAndRemoveIfEmtpy(uuid1);
  checkAndRemoveIfEmtpy(uuid2);

  return true;
}

//--------------------------------------------------------------------
bool ConnectionStorage::removeSegmentation(const PersistentSPtr segmentation)
{
  QWriteLocker lock(&m_lock);

  auto uuid = segmentation->uuid();

  if(!m_data.keys().contains(uuid)) return false;

  for(auto key: m_data[uuid].keys())
  {
    m_data[key].remove(uuid);
    if(m_data[key].isEmpty()) m_data.remove(key);

    m_data[uuid].remove(key);
  }

  m_data.remove(uuid);

  return true;
}

//--------------------------------------------------------------------
ConnectionStorage::Connections ConnectionStorage::connections(const PersistentSPtr segmentation) const
{
  QReadLocker lock(&m_lock);

  auto uuid = segmentation->uuid();
  ConnectionStorage::Connections result;

  if(!m_data.keys().contains(uuid)) return result;

  for(auto key: m_data[uuid].keys())
  {
    for(auto value: m_data[uuid][key])
    {
      ConnectionStorage::Connection connection;
      connection.segmentation1 = uuid;
      connection.segmentation2 = key;
      connection.point = NmVector3{value.x(), value.y(), value.z()};

      result << connection;
    }
  }

  return result;
}

//--------------------------------------------------------------------
ConnectionStorage::Connections ConnectionStorage::connections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2) const
{
  QReadLocker lock(&m_lock);

  auto uuid1 = segmentation1->uuid();
  auto uuid2 = segmentation2->uuid();
  ConnectionStorage::Connections result;

  if(!m_data.keys().contains(uuid1) || !m_data[uuid1].keys().contains(uuid2))
  {
    return result;
  }

  for(auto value: m_data[uuid1][uuid2])
  {
    ConnectionStorage::Connection connection;
    connection.segmentation1 = uuid1;
    connection.segmentation2 = uuid2;
    connection.point = NmVector3{value.x(), value.y(), value.z()};

    result << connection;
  }

  return result;
}

//--------------------------------------------------------------------
bool ConnectionStorage::save() const
{
  QReadLocker lock(&m_lock);

  if(m_data.isEmpty()) return false;

  if(!m_storage)
  {
    auto message = QObject::tr("No temporal storage defined.");
    auto details = QObject::tr("ConnectionStorage::save() -> ") + message;

    throw EspinaException(message, details);
  }

  QFile file{m_storage->absoluteFilePath(CONNECTIONS_STORAGE_FILE)};
  if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
  {
    auto message = QObject::tr("Couldn't open file %1 for writing.").arg(m_storage->absoluteFilePath(CONNECTIONS_STORAGE_FILE));
    auto details = QObject::tr("ConnectionStorage::save() -> ") + message;

    throw EspinaException(message, details);
  }

  QDataStream out(&file);
  out.setVersion(QDataStream::Version::Qt_4_8);
  out << m_data;
  file.close();

  return true;
}

//--------------------------------------------------------------------
bool ConnectionStorage::load()
{
  QWriteLocker lock(&m_lock);

  if(!m_storage)
  {
    auto message = QObject::tr("No temporal storage defined.");
    auto details = QObject::tr("ConnectionStorage::load() -> ") + message;

    throw EspinaException(message, details);
  }

  if(m_storage->exists(CONNECTIONS_STORAGE_FILE))
  {
    QFile file{m_storage->absoluteFilePath(CONNECTIONS_STORAGE_FILE)};
    if(!file.open(QIODevice::ReadOnly))
    {
      auto message = QObject::tr("Couldn't open file %1 for reading.").arg(m_storage->absoluteFilePath(CONNECTIONS_STORAGE_FILE));
      auto details = QObject::tr("ConnectionStorage::load() -> ") + message;

      throw EspinaException(message, details);
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Version::Qt_4_8);
    in >> m_data;

    return true;
  }

  return false;
}

//--------------------------------------------------------------------
void ConnectionStorage::checkAndRemoveIfEmtpy(const QString& uuid1, const QString& uuid2)
{
  if(!uuid2.isEmpty())
  {
    if(m_data[uuid1][uuid2].isEmpty())
    {
      m_data[uuid1].remove(uuid2);

      if(m_data[uuid1].isEmpty())
      {
        m_data.remove(uuid1);
      }
    }
  }
  else
  {
    if(m_data[uuid1].isEmpty())
    {
      m_data.remove(uuid1);
    }
  }
}

//--------------------------------------------------------------------
void ConnectionStorage::clear()
{
  m_data.clear();
}
