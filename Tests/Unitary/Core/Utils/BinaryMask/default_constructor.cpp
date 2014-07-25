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

#include <Core/Utils/BinaryMask.h>
#include "TestSupport.h"

using namespace ESPINA;

using BMask = BinaryMask<unsigned char>;

int default_constructor(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{-0.5,3.5,-0.5,3.5,-0.5,3.5};
  BMask mask(bounds);

  error |= (mask.bounds() != bounds);
  error |= (  0 != mask.backgroundValue());
  error |= (255 != mask.foregroundValue());

  mask.setForegroundValue(1);
  mask.setBackgroundValue(2);

  error |= ( 2 != mask.backgroundValue());
  error |= ( 1 != mask.foregroundValue());
  error |= (64 != mask.numberOfVoxels());

  BMask::const_iterator cit(&mask);
  while (!cit.isAtEnd())
  {
    error |= (cit.Get() != mask.backgroundValue());
    ++cit;
  }

  return error;
}
