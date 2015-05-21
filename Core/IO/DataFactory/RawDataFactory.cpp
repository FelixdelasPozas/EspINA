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
#include "RawDataFactory.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
DataSPtr RawDataFactory::createData(OutputSPtr           output,
                                    TemporalStorageSPtr  storage,
                                    const QString       &path,
                                    QXmlStreamAttributes info)
{
  DataSPtr data;

  const Data::Type requestedType = info.value("type").toString();
  const Bounds bounds(info.value("bounds").toString());

  if (!output->hasData(requestedType))
  {
    if (VolumetricData<itkVolumeType>::TYPE == requestedType)
    {
      data = std::make_shared<SparseVolume<itkVolumeType>>();
    }
    else if (MeshData::TYPE == requestedType)
    {
      data = std::make_shared<RawMesh>();
    }
    else if (SkeletonData::TYPE == requestedType)
    {
      data = std::make_shared<RawSkeleton>();
    }

    if (data)
    {
      data->setFetchContext(storage, path, QString::number(output->id()), bounds);
      output->setData(data);
    }
  }


  return data;
}
