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
#include "StorageFactory.h"

// Metadona
#include <IRODS_Storage.h>

// Qt
#include <QProcess>

using namespace ESPINA;

//----------------------------------------------------------------------------------------
Metadona::StorageSPtr StorageFactory::newStorage(Type type)
{
  // TODO: 17-06-2016 change for a generic, not Metadona.
  Metadona::StorageSPtr storage = nullptr;

  switch(type)
  {
    case StorageFactory::Type::IRODS:
      storage = std::make_shared<Metadona::IRODS::Storage>("metadona");
      break;
    default:
      break;
  }

  return storage;
}

//----------------------------------------------------------------------------------------
bool StorageFactory::supported(Type type)
{
  auto value = false;

  switch(type)
  {
    case StorageFactory::Type::IRODS:
      {
        QProcess process;
        process.start("ienv");
        if(!process.waitForFinished(1500))
        {
          value = false;
          process.kill();
        }
      }
      break;
    default:
      break;
  }

  return value;
}

//----------------------------------------------------------------------------------------
QList<StorageFactory::Type> StorageFactory::supportedStorages()
{
  QList<Type> supportedStorages;

  for(auto type: { Type::IRODS })
  {
    if(StorageFactory::supported(type))
    {
      supportedStorages << type;
    }
  }

  return supportedStorages;
}
