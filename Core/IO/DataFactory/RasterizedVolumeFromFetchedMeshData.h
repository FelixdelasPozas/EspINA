/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_RASTERIZED_VOLUME_FROM_FETCHED_MESH_DATA_H_
#define ESPINA_RASTERIZED_VOLUME_FROM_FETCHED_MESH_DATA_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/DataFactory.h>
#include <Core/Analysis/Data/MeshData.h>

namespace ESPINA
{
  class EspinaCore_EXPORT RasterizedVolumeFromFetchedMeshData
  : public DataFactory
  {
    public:
      virtual DataSPtr createData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info) override;

    protected:
      /** \brief Helper method to fetch a mesh from storate.
       *
       */
      virtual MeshDataSPtr fetchMeshData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path);
  };

} // namespace ESPINA

#endif // RASTERIZEDVOLUMEFROMFETCHEDMESHDATA_H_