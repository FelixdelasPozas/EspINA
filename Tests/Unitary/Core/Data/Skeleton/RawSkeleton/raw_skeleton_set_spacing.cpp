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

int raw_skeleton_set_spacing( int argc, char** argv )
{
  bool error = false;

  auto polyData = ESPINA::Testing::createSimpleTestSkeleton();
  NmVector3 spacing{1.0, 1.0, 1.0};
  auto output = std::make_shared<Output>(new DummyFilter(), 0, spacing);
  auto numberOfNodes = polyData->GetPoints()->GetNumberOfPoints();

  RawSkeleton skeleton{polyData, spacing, output};

  if(skeleton.spacing() != spacing)
  {
    std::cerr << "Unexpected skeleton spacing: " << skeleton.spacing() << ". Expected " << spacing << std::endl;
    error = true;
  }

  NmVector3 newSpacing{2.0,2.0,2.0};

  output->setSpacing(newSpacing);

  if(skeleton.spacing() != newSpacing)
  {
    std::cerr << "Unexpected skeleton spacing: " << skeleton.spacing() << " after changing it to " << newSpacing << std::endl;
    error = true;
  }

  Bounds defaultBounds = skeleton.bounds();

  if(defaultBounds != Bounds{0.0, 2.0, 0.0, 2.0, 0.0, 2.0})
  {
    std::cerr << "Scaled skeleton bounds: " << defaultBounds << ". Expected different bounds." << std::endl;
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

  if(skeleton.isEmpty())
  {
    std::cerr << "Skeleton musn't be empty." << std::endl;
    error = true;
  }

  if(numberOfNodes != skeleton.skeleton()->GetPoints()->GetNumberOfPoints())
  {
    std::cerr << "Unexpected number of points of scaled skeleton." << std::endl;
  }

  return error;
}
