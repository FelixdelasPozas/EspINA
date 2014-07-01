/*
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// Plugin
#include "SASFetchBehaviour.h"
#include "AppositionSurfaceFilter.h"

// EspINA
#include <Core/Analysis/Data/Volumetric/RasterizedVolume.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/EspinaTypes.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  MeshDataSPtr SASFetchBehaviour::fetchMeshData(OutputSPtr output,
                                                TemporalStorageSPtr storage,
                                                QString prefix)
  {
    MeshDataSPtr mesh = nullptr;

    if (!output->hasData(MeshData::TYPE))
    {
      auto data = DataSPtr{new RawMesh()};
      data->setOutput(output.get());

      if (data->fetchData(storage, prefix))
      {
        output->setData(data);

        // update filter values to avoid unnecessary calls to update().
        auto filter = dynamic_cast<AppositionSurfaceFilter *>(output->filter());
        Q_ASSERT(filter != nullptr);
        filter->m_alreadyFetchedData = true;
        filter->m_lastModifiedMesh = data->lastModified();
      }
    }

    mesh = meshData(output);

    return mesh;
  }

} // namespace EspINA

