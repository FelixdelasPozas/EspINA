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

#include <Core/Analysis/Data/Volumetric/RasterizedVolume.h>
#include <Core/IO/FetchBehaviour/RasterizedVolumeFromFetchedMeshData.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  void RasterizedVolumeFromFetchedMeshData::fetchOutputData(OutputSPtr output,
                                                            TemporalStorageSPtr storage,
                                                            QString prefix,
                                                            QXmlStreamAttributes info)
  {
    if ("MeshData" == info.value("type"))
    {
      fetchMeshData(output, storage, prefix);
    }
    else if ("VolumetricData" == info.value("type"))
    {
      auto data = DataSPtr {new SparseVolume<itkVolumeType>()};
      data->setOutput(output.get());
      if (data->fetchData(storage, prefix))
      {
        output->setData(data);
      }
      else
      {
        auto mesh = fetchMeshData(output, storage, prefix);
        auto spacing = mesh->spacing();
        auto bounds = mesh->bounds();

        if (mesh)
        {
          data = DataSPtr{new RasterizedVolume<itkVolumeType>(mesh, bounds, spacing)};
          output->setData(data);
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  MeshDataSPtr EspINA::RasterizedVolumeFromFetchedMeshData::fetchMeshData(OutputSPtr output,
                                                                          TemporalStorageSPtr storage,
                                                                          QString prefix)
  {
    MeshDataSPtr mesh;

    if (!output->hasData(MeshData::TYPE))
    {
      auto data = DataSPtr{new RawMesh()};
      data->setOutput(output.get());
      if (data->fetchData(storage, prefix))
      {
        output->setData(data);
        mesh = meshData(output);
      }
    }

    return mesh;
  }

} // namespace EspINA
