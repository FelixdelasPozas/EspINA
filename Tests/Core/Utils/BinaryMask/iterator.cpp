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
#include <Core/EspinaTypes.h>

using namespace ESPINA;

using BMask = BinaryMask<unsigned char>;

int iterator(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{-0.5,9.5,-0.5,9.5,-0.5,9.5};
  BMask mask(bounds);

  BMask::iterator it(&mask);

  ++it;
  it.goToBegin();

  try
  {
    --it;
    error |= true;
  }
  catch(BMask::Underflow_Exception const &e)
  {
    error |= false;
  }

  it.goToEnd();
  try
  {
    unsigned char test = it.Get();
    error |= true;
  }
  catch(BMask::Out_Of_Bounds_Exception const &e)
  {
    error |= false;
  }

  try
  {
    ++it;
    error |= true;
  }
  catch (BMask::Overflow_Exception const &e)
  {
    error |= false;
  }

  it.goToBegin();
  unsigned int count = 0;
  while(!it.isAtEnd())
  {
    error |= (mask.backgroundValue() != it.Get());
    it.Set();
    ++count;
    ++it;
  }

  error |= (count != mask.numberOfVoxels());
  error |= (count != 1000);

  it.goToBegin();
  while(!it.isAtEnd())
  {
    error |= (mask.foregroundValue() != it.Get());
    ++it;
  }

  return error;
}
