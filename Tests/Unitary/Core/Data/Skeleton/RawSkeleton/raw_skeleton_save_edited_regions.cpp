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

#include "Tests/Testing_Support.h"

#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Utils/vtkPolyDataUtils.h>

#include "SkeletonTestingUtils.h"

using namespace ESPINA;
using namespace ESPINA::Testing;
using namespace ESPINA::PolyDataUtils;

int raw_skeleton_save_edited_regions( int argc, char** argv )
{
  bool error = false;

  RawSkeleton skeleton;

  auto polyData = createSimpleTestSkeleton();

  skeleton.setSkeleton(polyData);

  if (skeleton.skeleton() != polyData)
  {
    std::cerr << "Unexpected skeleton polydata." << std::endl;
    error = true;
  }

  if (skeleton.editedRegions().size() != 1)
  {
    std::cerr << "Unexpected number of edited regions." << std::endl;
    error = true;
  }
  else
  {
    auto editedRegion = skeleton.editedRegions().first();

    if (editedRegion != skeleton.bounds())
    {
      std::cerr << "Unexpected edited region " << editedRegion << " differs from " << skeleton.bounds() << std::endl;
      error = true;
    }

    TemporalStorageSPtr storage{new TemporalStorage()};

    auto editedRegionSnapshots = skeleton.editedRegionsSnapshot(storage, "skeleton", "0");

    if (editedRegionSnapshots.size() != 1)
    {
      std::cerr << "Unexpected number of edited regions snapshots" << std::endl;
      error = true;
    }
    else
    {
      auto snapshot = editedRegionSnapshots.first();
      storage->saveSnapshot(snapshot);

      auto filename       = storage->absoluteFilePath(snapshot.first);
      auto savedSkeleton = readPolyDataFromFile(filename);

      if (savedSkeleton->GetNumberOfPoints() != polyData->GetNumberOfPoints())
      {
        std::cerr << "Unexpected number of points in saved skeleton." << std::endl;
        error = true;
      }

      if (savedSkeleton->GetNumberOfLines() != polyData->GetNumberOfLines())
      {
        std::cerr << "Unexpected number of lines in saved skeleton." << std::endl;
        error = true;
      }
    }
  }

  return error;
}


