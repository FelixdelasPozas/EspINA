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

// VTK
#include <vtkPolyData.h>

namespace ESPINA
{
  const Data::Type SkeletonData::TYPE = "SkeletonData";

  //----------------------------------------------------------------------------
  SkeletonData::SkeletonData()
  {
  }
  
  //----------------------------------------------------------------------------
  Bounds SkeletonData::bounds() const
  {
    Nm bounds[6]{0,-1,0,-1,0,-1};

    auto data = this->skeleton();
    if(data != nullptr)
      data->GetBounds(bounds);

    return Bounds{ bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
  }

  //----------------------------------------------------------------------------
  bool SkeletonData::fetchData()
  {
    return fetchData(m_storage, m_path, m_id);
  }

  //----------------------------------------------------------------------------
  bool SkeletonData::fetchData(TemporalStorageSPtr storage, const QString& path, const QString& id) const
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
  Snapshot SkeletonData::snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
  {
    Snapshot snapshot;

    auto currentSkeleton = skeleton();
    if (currentSkeleton)
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
    return DataSPtr{new SkeletonProxy()};
  }

  //----------------------------------------------------------------------------
  bool hasSkeletonData(OutputSPtr output)
  {
    return output->hasData(SkeletonData::TYPE);
  }

  //----------------------------------------------------------------------------
  SkeletonDataSPtr skeletonData(OutputSPtr output)
  {
    SkeletonDataSPtr skeletonData = std::dynamic_pointer_cast<SkeletonData>(output->data(SkeletonData::TYPE));

    return skeletonData;
  }

} // namespace EspINA
