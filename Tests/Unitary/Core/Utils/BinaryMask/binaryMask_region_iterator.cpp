/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

    This program is free software: you can redistribute it and/or modify
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
#include <Core/Utils/Bounds.h>
#include <Core/EspinaTypes.h>

using namespace EspINA;

using BMask = BinaryMask<unsigned char>;

int binaryMask_region_iterator(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{ 0,4,0,4,0,4 };
  BMask *mask = new BMask(bounds);

  Bounds badRegionBounds{ -1,18,-1,18,-1,18 };

  try
  {
    BMask::region_iterator badIt(mask, badRegionBounds);
    error |= true;
  }
  catch(BMask::Region_Not_Contained_In_Mask const &e)
  {
    error |= false;
  }

  Bounds goodRegionBounds{ 1,3,1,3,1,3 };
  BMask::region_iterator rit(mask, goodRegionBounds);

  rit.goToEnd();
  unsigned char test;
  try
  {
    test = rit.Get();
    error |= true;
  }
  catch(BMask::Out_Of_Bounds_Exception const &e)
  {
    error |= false;
  }

  try
  {
    ++rit;
    error |= true;
  }
  catch (BMask::Overflow_Exception const &e)
  {
    error |= false;
  }

  rit.goToBegin();
  try
  {
    --rit;
    error |= true;
  }
  catch(BMask::Underflow_Exception const &e)
  {
    error |= false;
  }

  rit.goToBegin();
  unsigned int count = 0;
  while(!rit.isAtEnd())
  {
    error |= (mask->backgroundValue() != rit.Get());
    rit.Set();
    ++count;
    ++rit;
  }

  error |= (count != 27);

  rit.goToBegin();
  while(!rit.isAtEnd())
  {
    error |= (mask->foregroundValue() != rit.Get());
    ++rit;
  }

  return error;
}
