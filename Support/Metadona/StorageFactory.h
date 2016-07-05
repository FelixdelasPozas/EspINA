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

#ifndef ESPINA_METADATA_STORAGE_FACTORY_H
#define ESPINA_METADATA_STORAGE_FACTORY_H

#include "Support/EspinaSupport_Export.h"

// Metadona
#include <Storage.h>

// Qt
#include <QList>

namespace ESPINA
{
  class EspinaSupport_EXPORT StorageFactory
  {
    public:
      enum class Type: char { IRODS = 0 };
      /** \brief Creates and returns a new storage.
       * \param[in] type type of storage
       *
       */
      static Metadona::StorageSPtr newStorage(Type type);

      /** \brief Returns true if the storage is correctly configured and can be used and false otherwise.
       * \param[in] type type of storage to test for support.
       *
       */
      static bool supported(Type type);

      /** \brief Returns a list of supported storages.
       *
       */
      static QList<Type> supportedStorages();
  };
} // namespace ESPINA

#endif // ESPINA_METADATA_STORAGE_FACTORY_H
