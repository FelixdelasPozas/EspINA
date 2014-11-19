/*
 * Copyright 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// ESPINA
#include "MarchingCubesFromFetchedVolumetricData.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>

using namespace ESPINA;

//----------------------------------------------------------------------------
DataSPtr MarchingCubesFromFetchedVolumetricData::createData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info)
{
  DataSPtr data;

  if ("VolumetricData" == info.value("type"))
  {
    data = createVolumetricData(output, storage, path);
  }
  else if ("MeshData" == info.value("type"))
  {
    if (!hasMeshData(output))
    {
      auto volume = createVolumetricData(output, storage, path);
      Q_ASSERT(volume);
      auto data = DataSPtr{new MarchingCubesMesh<itkVolumeType>(volume)};
      data->setFetchContext(storage, path, QString::number(output->id()));
      output->setData(data);
    }

    data = meshData(output, DataUpdatePolicy::Ignore);
  }

  return data;
}

//----------------------------------------------------------------------------
ESPINA::DefaultVolumetricDataSPtr MarchingCubesFromFetchedVolumetricData::createVolumetricData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path)
{
  if (!output->hasData(VolumetricData<itkVolumeType>::TYPE))
  {
    auto data = DataSPtr{new SparseVolume<itkVolumeType>()};
    data->setFetchContext(storage, path, QString::number(output->id()));
    output->setData(data);
  }

  return volumetricData(output, DataUpdatePolicy::Ignore);
}
