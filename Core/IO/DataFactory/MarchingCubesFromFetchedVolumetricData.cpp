/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "Core/Analysis/Data.h"
#include "MarchingCubesFromFetchedVolumetricData.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
DataSPtr MarchingCubesFromFetchedVolumetricData::createData(OutputSPtr output, TemporalStorageSPtr storage, const QString &path, QXmlStreamAttributes info)
{
  DataSPtr data;

  const Data::Type requestedType = info.value("type").toString();
  VolumeBounds bounds(Bounds(info.value("bounds").toString()), output->spacing());

  if (VolumetricData<itkVolumeType>::TYPE == requestedType)
  {
    data = createVolumetricData(output, storage, path, bounds);
  }
  else if (MeshData::TYPE == requestedType)
  {
    data = createMeshData(output, storage, path, bounds);
  }
  else if(SkeletonData::TYPE == requestedType)
  {
    data = createSkeletonData(output, storage, path, bounds);
  }
  else
  {
    auto message = QObject::tr("Unknown data type for data factory: %1").arg(requestedType);
    auto details = QObject::tr("MarchingCubesFromFetchedVolumetricData::createData() -> ") + message;

    throw EspinaException(message, details);
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

//----------------------------------------------------------------------------
SkeletonDataSPtr MarchingCubesFromFetchedVolumetricData::createSkeletonData(OutputSPtr          output,
                                                                            TemporalStorageSPtr storage,
                                                                            const QString      &path,
                                                                            const VolumeBounds &bounds)
{
  if(!output->hasData(SkeletonData::TYPE))
  {
    auto data = std::make_shared<RawSkeleton>(bounds.spacing(), bounds.origin());
    data->setFetchContext(storage, path, QString::number(output->id()), bounds);

    output->setData(data);
  }

  return writeLockSkeleton(output, DataUpdatePolicy::Ignore);
}
