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

#include <QList>

using ESPINA::Testing::DummyFilter;

using namespace ESPINA;

int raw_skeleton_constructor( int argc, char** argv )
{
  bool error = false;

  auto polyData = ESPINA::Testing::createSimpleTestSkeleton();
  NmVector3 spacing{1.0, 1.0, 1.0};
  auto output = std::make_shared<Output>(new DummyFilter(), 0, spacing);

  auto skeleton = std::make_shared<RawSkeleton>(polyData, spacing, output);

  Bounds defaultBounds = skeleton->bounds();

  if (!defaultBounds.areValid())
  {
    std::cerr << "Constructed skeleton bounds: " << defaultBounds << ". Expected valid bounds." << std::endl;
    error = true;
  }

  if (defaultBounds != Bounds{-0.5, 1.5, -0.5, 1.5, -0.5, 1.5})
  {
    std::cerr << "Constructed skeleton bounds: " << defaultBounds << ". Expected different bounds." << std::endl;
    error = true;
  }

  if(skeleton->spacing() != NmVector3{1,1,1})
  {
    std::cerr << "Unexpected skeleton spacing." << std::endl;
    error = true;
  }

  for (Axis dir : { Axis::X, Axis::Y, Axis::Z })
  {
    if (!defaultBounds.areLowerIncluded(dir))
    {
      std::cerr << "Default skeleton bounds must have lower bounds included" << std::endl;
      error = true;
    }

    if (defaultBounds.areUpperIncluded(dir))
    {
      std::cerr << "Default skeleton bounds must have upper bounds excluded" << std::endl;
      error = true;
    }
  }

  if (skeleton->memoryUsage() == 0)
  {
    std::cerr << "Default skeleton must use some memory to store polyData." << std::endl;
    error = true;
  }

  if (skeleton->isValid() != true)
  {
    std::cerr << "Default skeleton must be valid." << std::endl;
    error = true;
  }

  if(skeleton->isEdited() != false)
  {
    std::cerr << "Skeleton edited regions must be empty." << std::endl;
    error = true;
  }

  if(skeleton->dependencies() != QList<Data::Type>())
  {
    std::cerr << "RawSkeleton musn't have data dependencies." << std::endl;
  }

  if(skeleton->type() != SkeletonData::TYPE)
  {
    std::cerr << "RawSkeleton must return SkeletonData as type." << std::endl;
  }

  if(skeleton->isEmpty())
  {
    std::cerr << "Skeleton musn't be empty." << std::endl;
    error = true;
  }

  output->setData(skeleton);

  if(hasSkeletonData(output) != true)
  {
    std::cerr << "Output must have an skeleton data." << std::endl;
    error = true;
  }

  auto skeletondata = ESPINA::skeletonData(output);
  if(skeletondata->skeleton() != skeleton->skeleton())
  {
    std::cerr << "Unexpected SkeletonData in output." << std::endl;
    error = true;
  }

  return error;
}
