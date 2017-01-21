/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonData.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Skeleton/SkeletonProxy.h>
#include <Core/Utils/vtkPolyDataUtils.h>
#include <Core/Utils/VolumeBounds.h>

// VTK
#include <vtkPolyData.h>

namespace ESPINA
{
  const Data::Type SkeletonData::TYPE = "SkeletonData";

  //----------------------------------------------------------------------------
  SkeletonData::SkeletonData()
  {
  }
  
//   //----------------------------------------------------------------------------
//   Bounds SkeletonData::bounds() const
//   {
//     Bounds result;
//
//     auto skeletonData = this->skeleton();
//
//     if(skeletonData != nullptr)
//     {
//       Nm bounds[6];
//
//       skeletonData->GetBounds(bounds);
//
//       Bounds polyDataBounds{bounds[0], bounds[1], bounds[2],
//                             bounds[3], bounds[4], bounds[5]};
//
//       result = VolumeBounds{polyDataBounds, spacing(), NmVector3{0,0,0}}.bounds();
//     }
//
//     return result;
//   }

  //----------------------------------------------------------------------------
  bool SkeletonData::fetchDataImplementation(TemporalStorageSPtr storage, const QString& path, const QString& id, const VolumeBounds &bounds)
  {
    bool dataFetched = false;

    QFileInfo skeletonFile(storage->absoluteFilePath(snapshotFilename(path, id)));

    if(skeletonFile.exists())
    {
      setSkeleton(PolyDataUtils::readPolyDataFromFile(skeletonFile.absoluteFilePath()));
      dataFetched = true;
    }

    return dataFetched;
  }

  //----------------------------------------------------------------------------
  Snapshot SkeletonData::snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    Snapshot snapshot;

    auto currentSkeleton = skeleton();
    if (currentSkeleton != nullptr)
    {
      QString fileName = snapshotFilename(path, id);
      storage->makePath(path);

      snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(currentSkeleton));
    }

    return snapshot;
  }

  //----------------------------------------------------------------------------
  DataSPtr SkeletonData::createProxy() const
  {
    return std::make_shared<SkeletonProxy>();
  }

  //----------------------------------------------------------------------------
  bool hasSkeletonData(OutputSPtr output)
  {
    return output->hasData(SkeletonData::TYPE);
  }

  //----------------------------------------------------------------------------
  Output::ReadLockData<SkeletonData> readLockSkeleton(OutputSPtr output, DataUpdatePolicy policy)
  {
    return outputReadLockData<SkeletonData>(output.get(), policy);
  }

  //----------------------------------------------------------------------------
  Output::ReadLockData<SkeletonData> readLockSkeleton(Output *output, DataUpdatePolicy policy)
  {
    return outputReadLockData<SkeletonData>(output, policy);
  }

  //----------------------------------------------------------------------------
  Output::WriteLockData<SkeletonData> writeLockSkeleton(Output *output, DataUpdatePolicy policy)
  {
    return outputWriteLockData<SkeletonData>(output, policy);
  }

  //----------------------------------------------------------------------------
  Output::WriteLockData<SkeletonData> writeLockSkeleton(OutputSPtr output, DataUpdatePolicy policy)
  {
    return outputWriteLockData<SkeletonData>(output.get(), policy);
  }

} // namespace EspINA
