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

#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/Bounds.h>
#include <Core/Types.h>

using namespace ESPINA;

using BMask = BinaryMask<unsigned char>;

int const_iterator(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{ 0,9,0,9,0,9 };
  BMask mask(bounds);

  BMask::const_iterator cit(&mask);

  // the rest have been tested in non-const iterator
  cit.goToEnd();
  try
  {
    cit.Set();
    error |= true;
  }
  catch (BMask::Const_Violation_Exception const &e)
  {
    error |= false;
  }

  try
  {
    cit.Unset();
    error |= true;
  }
  catch (BMask::Const_Violation_Exception const &e)
  {
    error |= false;
  }

  return error;
}

