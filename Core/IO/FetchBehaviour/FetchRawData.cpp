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
#include "FetchRawData.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
DataSPtr FetchRawData::fetchOutputData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info)
{
  DataSPtr data;

  if ("VolumetricData" == info.value("type"))
  {
    if (!output->hasData(VolumetricData<itkVolumeType>::TYPE))
    {
      data = DataSPtr{new SparseVolume<itkVolumeType>()};
      data->setOutput(output.get());
      if (data->fetchData(storage, path, QString::number(output->id())))
      {
        output->setData(data);
      }
    }
  }
  else if ("MeshData" == info.value("type"))
  {
    if (!output->hasData(MeshData::TYPE))
    {
      data = DataSPtr{new RawMesh()};
      data->setOutput(output.get());
      if (data->fetchData(storage, path, QString::number(output->id())))
      {
        output->setData(data);
      }
    }
  }
  else if ("SkeletonData" == info.value("type"))
  {
    if (!output->hasData(SkeletonData::TYPE))
    {
      auto data = DataSPtr {new RawSkeleton(output)};
      if (data->fetchData(storage, path, QString::number(output->id())))
      {
        output->setData(data);
      }
    }
  }

  return data;
}
