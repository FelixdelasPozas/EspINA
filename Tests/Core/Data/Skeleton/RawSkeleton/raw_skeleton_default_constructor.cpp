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

#include <QList>

using namespace ESPINA;

int raw_skeleton_default_constructor( int argc, char** argv )
{
  bool error = false;

  RawSkeleton skeleton;

  Bounds defaultBounds = skeleton.bounds();

  if (defaultBounds.areValid())
  {
    std::cerr << "Default constructed skeleton bounds: " << defaultBounds << ". Expected invalid bounds." << std::endl;
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

  if (skeleton.memoryUsage() != 0)
  {
    std::cerr << "Default skeleton memory usage must be 0" << std::endl;
    error = true;
  }

  if (skeleton.isValid() != false)
  {
    std::cerr << "Default skeleton data must be empty" << std::endl;
    error = true;
  }

  if(skeleton.isEdited() != false)
  {
    std::cerr << "Default skeleton edited regions must be empty." << std::endl;
    error = true;
  }

  if(skeleton.dependencies() != QList<Data::Type>())
  {
    std::cerr << "RawSkeleton musn't have data dependencies." << std::endl;
  }

  if(skeleton.type() != SkeletonData::TYPE)
  {
    std::cerr << "RawSkeleton must return SkeletonData as type." << std::endl;
  }

  return error;
}

