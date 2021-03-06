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

#include "Core/Analysis/Data/Skeleton/RawSkeleton.h"
#include <Core/Utils/Spatial.h>
#include <Tests/testing_support_dummy_filter.h>


#include "SkeletonTestingUtils.h"

using ESPINA::Testing::DummyFilter;

using namespace ESPINA;

int raw_skeleton_set_skeleton( int argc, char** argv )
{
  bool error = false;

  auto polyData      = ESPINA::Testing::createSimpleTestSkeleton();
  auto numberOfNodes = polyData->GetPoints()->GetNumberOfPoints();

  NmVector3   spacing{1.0, 1.0, 1.0};
  RawSkeleton skeleton{polyData, spacing};

  if(skeleton.editedRegions() != BoundsList())
  {
    std::cerr << "Unexpected edited regions in unmodified skeleton." << std::endl;
    error = true;
  }

  if(skeleton.isEdited() != false)
  {
    std::cerr << "Unedited skeleton must not be edited." << std::endl;
    error = true;
  }

  if(skeleton.skeleton()->GetPoints()->GetNumberOfPoints() != numberOfNodes)
  {
    std::cerr << "Unexpected number of points of test skeleton." << std::endl;
    error = true;
  }

  int  newNumberOfNodes = 5;
  auto newPolyData      = ESPINA::Testing::createRandomTestSkeleton(newNumberOfNodes);

  Nm bounds[6];
  newPolyData->GetBounds(bounds);
  auto newPolyDataBounds = Bounds{ std::roundf(bounds[0]/spacing[0])-spacing[0]/2,
                                   std::roundf(bounds[1]/spacing[0])+spacing[0]/2,
                                   std::roundf(bounds[2]/spacing[1])-spacing[1]/2,
                                   std::roundf(bounds[3]/spacing[1])+spacing[1]/2,
                                   std::roundf(bounds[4]/spacing[2])-spacing[2]/2,
                                   std::roundf(bounds[5]/spacing[2])+spacing[2]/2};

  skeleton.setSkeleton(newPolyData);

  if(skeleton.skeleton()->GetPoints()->GetNumberOfPoints() != newNumberOfNodes)
  {
    std::cerr << "Unexpected number of points of test skeleton." << std::endl;
    error = true;
  }

  if(skeleton.isEdited() != true)
  {
    std::cerr << "Modified skeleton must have edited regions.";
    error = true;
  }

  if(skeleton.bounds() != newPolyDataBounds)
  {
    std::cerr << "Unexpected bounds " << skeleton.bounds() << " expecting " << newPolyDataBounds << std::endl;
    error = true;
  }

  auto editedRegion = skeleton.editedRegions().first();
  if(editedRegion != newPolyDataBounds)
  {
    std::cerr << "Unexpected edited region " << editedRegion << " expecting " << newPolyDataBounds << std::endl;
    error = true;
  }

  return error;
}
