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

// Plugin
#include "SASFetchBehaviour.h"
#include "AppositionSurfaceFilter.h"

// ESPINA
#include <Core/Analysis/Data/Volumetric/RasterizedVolume.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/EspinaTypes.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  MeshDataSPtr SASFetchBehaviour::fetchMeshData(OutputSPtr output,
                                                TemporalStorageSPtr storage,
                                                const QString &path)
  {
    MeshDataSPtr mesh = nullptr;

    // TODO BUG WARNING
//     if (!output->hasData(MeshData::TYPE))
//     {
//       auto data = DataSPtr{new RawMesh()};
//       data->setOutput(output.get());
//
//       if (data->fetchData(storage, path, QString::number(output->id())))
//       {
//         output->setData(data);
//
//         // update filter values to avoid unnecessary calls to update().
//         auto filter = dynamic_cast<AppositionSurfaceFilter *>(output->filter());
//         Q_ASSERT(filter != nullptr);
//         filter->m_alreadyFetchedData = true;
//         filter->m_lastModifiedMesh = data->lastModified();
//       }
//     }
//
//     mesh = meshData(output);

    return mesh;
  }

} // namespace ESPINA

