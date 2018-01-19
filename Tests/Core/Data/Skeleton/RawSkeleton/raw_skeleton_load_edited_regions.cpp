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

int raw_skeleton_load_edited_regions( int argc, char** argv )
{
  bool error = false;

  RawSkeleton skeleton;

  auto polyData = createRandomTestSkeleton(20);

  skeleton.setSkeleton(polyData);

  if (skeleton.editedRegions().size() != 1)
  {
    std::cerr << "Unexpected number of edited regions" << std::endl;
    error = true;
  }
  else
  {
    auto editedRegions = skeleton.editedRegions();
    auto editedRegion  = editedRegions.first();

    if (editedRegion != skeleton.bounds())
    {
      std::cerr << "Unexpected edited region " << editedRegion << " differs from " << skeleton.bounds() << std::endl;
      error = true;
    }

    auto storage = std::make_shared<TemporalStorage>();

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
    }

    skeleton.setSkeleton(nullptr); // Clear skeleton
    skeleton.setEditedRegions(editedRegions);// restore expected edited regions

    if (skeleton.skeleton() != nullptr)
    {
      std::cerr << "Unexpected skeleton polydata after data clear." << std::endl;
      error = true;
    }

    skeleton.restoreEditedRegions(storage, "skeleton", "0");

    if (editedRegions != skeleton.editedRegions())
    {
      std::cerr << "Edited regions shouldn't be modified after restoration." << std::endl;
      error = true;
    }

    auto restoredSkeleton = skeleton.skeleton();

    if (restoredSkeleton->GetNumberOfPoints() != polyData->GetNumberOfPoints())
    {
      std::cerr << "Unexpected number of points in restored skeleton." << std::endl;
      error = true;
    }

    if(restoredSkeleton->GetNumberOfLines() != polyData->GetNumberOfLines())
    {
      std::cerr << "Unexpected number of lines in restored skeleton." << std::endl;
      error = true;
    }

  }

  return error;
}



