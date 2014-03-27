/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <QDebug>

using namespace EspINA;

using BMask = BinaryMask<unsigned char>;

int region_iterator(int argc, char** argv)
{
  bool error = false;

  BMask mask(Bounds{ -0.5,3.5,-0.5,3.5,-0.5,3.5 });

  Bounds badRegionBounds{ -1,18,-1,18,-1,18 };

  try
  {
    BMask::region_iterator badIt(&mask, badRegionBounds);
    error |= true;
  }
  catch(BMask::Region_Not_Contained_In_Mask_Exception const &e)
  {
    error |= false;
  }

  Bounds goodRegionBounds{ 0.5,3.5,1.5,2.5,1.5,3.5 };
  BMask::region_iterator rit(&mask, goodRegionBounds);

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

  mask.setForegroundValue(1);
  rit.goToBegin();
  unsigned int count = 0;
  QString inputValues, outputValues;
  while(!rit.isAtEnd())
  {
    error |= (mask.backgroundValue() != rit.Get());
    rit.Set();
    inputValues += QString::number(rit.Get());
    ++count;
    ++rit;
  }

  error |= (count != 6);

  rit.goToBegin();
  while(!rit.isAtEnd())
  {
    error |= (mask.foregroundValue() != rit.Get());
    outputValues += QString::number(rit.Get());
    ++rit;
  }

  BMask otherMask(Bounds{0,8.75,0,8.75,0,8.75}, NmVector3{2.5,2.5,2.5});

  error |= (otherMask.numberOfVoxels() != 64);

  error |= (outputValues != inputValues);

  return error;
}
