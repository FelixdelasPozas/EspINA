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
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//---------------------------------------------------------------------------
DataSPtr RasterizedVolumeFromFetchedMeshData::createData(OutputSPtr           output,
                                                         TemporalStorageSPtr  storage,
                                                         const QString       &path,
                                                         QXmlStreamAttributes info)
{
  const Data::Type requestedType = info.value("type").toString();
  VolumeBounds bounds(Bounds(info.value("bounds").toString()), output->spacing());

  DataSPtr data;
  if(MeshData::TYPE == requestedType)
  {
    data = createMeshData(output, storage, path, bounds);
  }
  else if(VolumetricData<itkVolumeType>::TYPE == requestedType)
  {
    data = createVolumetricData(output, storage, path, bounds);
  }
  else if(SkeletonData::TYPE == requestedType)
  {
    data = createSkeletonData(output, storage, path, bounds);
  }
  else
  {
    auto message = QObject::tr("Unknown data type for data factory: %1").arg(requestedType);
    auto details = QObject::tr("RasterizedVolumeFromFetchedMeshData::createData() -> ") + message;

    throw EspinaException(message, details);
  }

  return data;
}

//----------------------------------------------------------------------------
MeshDataSPtr RasterizedVolumeFromFetchedMeshData::createMeshData(OutputSPtr          output,
                                                                 TemporalStorageSPtr storage,
                                                                 const QString      &path,
                                                                 const VolumeBounds &bounds)
{
  if (!hasMeshData(output))
  {
    auto data = std::make_shared<RawMesh>();

    data->setFetchContext(storage, path, QString::number(output->id()), bounds);
    output->setData(data);
  }

  auto mesh = writeLockMesh(output, DataUpdatePolicy::Ignore);

  return mesh;
}

//----------------------------------------------------------------------------
DefaultVolumetricDataSPtr RasterizedVolumeFromFetchedMeshData::createVolumetricData(OutputSPtr          output,
                                                                                    TemporalStorageSPtr storage,
                                                                                    const QString      &path,
                                                                                    const VolumeBounds &bounds)
{
  if(!hasVolumetricData(output))
  {
    auto data = std::make_shared<SparseVolume<itkVolumeType>>();

    output->setData(data);
    data->setFetchContext(storage, path, QString::number(output->id()), bounds);

    if (!data->fetchData())
    {
      auto mesh       = createMeshData(output, storage, path, bounds);
      auto meshBounds = mesh->bounds();
      auto spacing    = meshBounds.spacing();

      if (mesh)
      {
        data = std::make_shared<RasterizedVolume<itkVolumeType>>(output.get(), meshBounds, spacing);
      }
    }
  }

  return writeLockVolume(output, DataUpdatePolicy::Ignore);
}

//----------------------------------------------------------------------------
SkeletonDataSPtr RasterizedVolumeFromFetchedMeshData::createSkeletonData(OutputSPtr          output,
                                                                         TemporalStorageSPtr storage,
                                                                         const QString      &path,
                                                                         const VolumeBounds &bounds)
{
  if(!hasSkeletonData(output))
  {
    auto data = std::make_shared<RawSkeleton>(bounds.spacing(), bounds.origin());
    data->setFetchContext(storage, path, QString::number(output->id()), bounds);

    output->setData(data);
  }

  return writeLockSkeleton(output, DataUpdatePolicy::Ignore);
}
