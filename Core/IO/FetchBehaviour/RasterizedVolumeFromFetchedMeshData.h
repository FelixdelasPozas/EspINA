/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include <Core/Analysis/FetchBehaviour.h>
#include "Core/Analysis/Data/MeshData.h"

namespace EspINA
{
  class RasterizedVolumeFromFetchedMeshData
  : public FetchBehaviour
  {
    public:
      virtual void fetchOutputData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix, QXmlStreamAttributes info);

    private:
      MeshDataSPtr fetchMeshData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix);
  };

} // namespace EspINA

#endif // RASTERIZEDVOLUMEFROMFETCHEDMESHDATA_H_
