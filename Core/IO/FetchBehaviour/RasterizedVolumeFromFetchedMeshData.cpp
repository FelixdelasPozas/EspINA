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

// ESPINA
#include "RasterizedVolumeFromFetchedMeshData.h"
#include <Core/Analysis/Data/Volumetric/RasterizedVolume.hxx>

namespace ESPINA
{
  //---------------------------------------------------------------------------
  DataSPtr RasterizedVolumeFromFetchedMeshData::createData(OutputSPtr           output,
                                                           TemporalStorageSPtr  storage,
                                                           const QString       &path,
                                                           QXmlStreamAttributes info)
  {
    DataSPtr data;

    if ("MeshData" == info.value("type"))
    {
      data = fetchMeshData(output, storage, path);
    }
    else if ("VolumetricData" == info.value("type"))
    {
      data = std::make_shared<SparseVolume<itkVolumeType>>();

      output->setData(data);

      data->setFetchContext(storage, path, QString::number(output->id()));
      if (!data->fetchData())
      {
        auto mesh    = fetchMeshData(output, storage, path);
        auto spacing = mesh->spacing();
        auto bounds  = mesh->bounds();

        if (mesh)
        {
          data = std::make_shared<RasterizedVolume<itkVolumeType>>(mesh, bounds, spacing);
        }
      }
    }


    return data;
  }

  //----------------------------------------------------------------------------
  MeshDataSPtr ESPINA::RasterizedVolumeFromFetchedMeshData::fetchMeshData(OutputSPtr          output,
                                                                          TemporalStorageSPtr storage,
                                                                          const QString      &path)
  {
    MeshDataSPtr data;

    if (!hasMeshData(output))
    {
      data = std::make_shared<RawMesh>();

      data->setFetchContext(storage, path, QString::number(output->id()));
      output->setData(data);
    }

    data = meshData(output, DataUpdatePolicy::Ignore);

    return data;
  }

} // namespace ESPINA
