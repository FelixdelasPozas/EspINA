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
#include "Core/Analysis/Data.h"
#include "MarchingCubesFromFetchedVolumetricData.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
DataSPtr MarchingCubesFromFetchedVolumetricData::createData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info)
{
  DataSPtr data;

  VolumeBounds bounds(Bounds(info.value("bounds").toString()), output->spacing());

  if ("VolumetricData" == info.value("type"))
  {
    data = createVolumetricData(output, storage, path, bounds);
  }
  else if ("MeshData" == info.value("type"))
  {
    data = createMeshData(output, storage, path, bounds);
  }

  return data;
}

//----------------------------------------------------------------------------
DefaultVolumetricDataSPtr MarchingCubesFromFetchedVolumetricData::createVolumetricData(OutputSPtr          output,
                                                                                       TemporalStorageSPtr storage,
                                                                                       const QString      &path,
                                                                                       const VolumeBounds &bounds)
{
  if (!output->hasData(VolumetricData<itkVolumeType>::TYPE))
  {
    // NOTE: As a consequence of passing actual volume bounds stored in the output file
    //       we could try to remove bounds from fetch context API as needFetch/fetchBounds workaround
    auto data = std::make_shared<SparseVolume<itkVolumeType>>(bounds);
    data->setFetchContext(storage, path, QString::number(output->id()), bounds);

    output->setData(data);
  }

  return writeLockVolume(output, DataUpdatePolicy::Ignore);
}

//----------------------------------------------------------------------------
MeshDataSPtr MarchingCubesFromFetchedVolumetricData::createMeshData(OutputSPtr          output,
                                                                    TemporalStorageSPtr storage,
                                                                    const QString      &path,
                                                                    const VolumeBounds &bounds)
{
  if (!output->hasData(MeshData::TYPE))
  {
    createVolumetricData(output, storage, path, bounds);

    auto data = std::make_shared<MarchingCubesMesh>(output.get());
    data->setFetchContext(storage, path, QString::number(output->id()), bounds);

    output->setData(data);
  }

  return writeLockMesh(output, DataUpdatePolicy::Ignore);
}
