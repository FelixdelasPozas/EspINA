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
#include "RawDataFactory.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
DataSPtr RawDataFactory::createData(OutputSPtr           output,
                                    TemporalStorageSPtr  storage,
                                    const QString       &path,
                                    QXmlStreamAttributes info)
{
  DataSPtr data;

  const Data::Type requestedType = info.value("type").toString();
  const VolumeBounds bounds(Bounds(info.value("bounds").toString()), output->spacing());

  if (!output->hasData(requestedType))
  {
    if(VolumetricData<itkVolumeType>::TYPE == requestedType)
    {
      data = std::make_shared<SparseVolume<itkVolumeType>>(bounds);
    }
    else
    {
      if(MeshData::TYPE == requestedType)
      {
        data = std::make_shared<RawMesh>();
      }
      else
      {
        if(SkeletonData::TYPE == requestedType)
        {
          data = std::make_shared<RawSkeleton>(bounds.spacing(), bounds.origin());
        }
        else
        {
          auto message = QObject::tr("Unknown data type for RawDataFactory: %1").arg(requestedType);
          auto details = QObject::tr("RawDataFactory::createData() -> ") + message;

          throw EspinaException(message, details);
        }
      }
    }

    if (data)
    {
      data->setFetchContext(storage, path, QString::number(output->id()), bounds);
      output->setData(data);
    }
  }

  return data;
}
