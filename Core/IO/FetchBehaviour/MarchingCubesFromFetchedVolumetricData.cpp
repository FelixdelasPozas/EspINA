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
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>

using namespace ESPINA;

//----------------------------------------------------------------------------
void MarchingCubesFromFetchedVolumetricData::fetchOutputData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix, QXmlStreamAttributes info)
{
  if ("VolumetricData" == info.value("type"))
  {
    fetchVolumetricData(output, storage, prefix);
  }
  else if ("MeshData" == info.value("type"))
  {
    auto data = DataSPtr { new RawMesh() };
    data->setOutput(output.get());
    if (data->fetchData(storage, prefix))
    {
      output->setData(data);
    }
    else
    {
      auto volume = fetchVolumetricData(output, storage, prefix);
      if (volume)
      {
        data = DataSPtr{new MarchingCubesMesh<itkVolumeType>(volume)};
        output->setData(data);
      }
    }
  }
}

//----------------------------------------------------------------------------
ESPINA::DefaultVolumetricDataSPtr MarchingCubesFromFetchedVolumetricData::fetchVolumetricData(OutputSPtr output, TemporalStorageSPtr storage, QString prefix)
{
  DefaultVolumetricDataSPtr volume = nullptr;

  if (!output->hasData(VolumetricData<itkVolumeType>::TYPE))
  {
    auto data = DataSPtr { new SparseVolume<itkVolumeType>() };
    data->setOutput(output.get());
    if (data->fetchData(storage, prefix))
    {
      output->setData(data);
    }
  }

  volume = volumetricData(output);

  return volume;
}
