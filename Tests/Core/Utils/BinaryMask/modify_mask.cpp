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

#include <itkImageRegionConstIterator.h>

using namespace ESPINA;

using BMask = BinaryMask<unsigned char>;

int modify_mask(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{-0.5,3.5,-0.5,3.5,-0.5,3.5};
  BMask mask(bounds);

  BMask::iterator cit(&mask);
  cit.goToBegin();
  unsigned int i = 0;
  while(!cit.isAtEnd())
  {
    if (i % 2)
      cit.Set();

    ++cit;
    ++i;
  }

  cit.goToBegin();
  i = 0;
  QString volumeBits;
  while (!cit.isAtEnd())
  {
    try
    {
      if (i % 2)
      {
        error |= (mask.foregroundValue() != cit.Get());
        volumeBits += "1";
      }
      else
      {
        error |= (mask.backgroundValue() != cit.Get());
        volumeBits += "0";
      }
    }
    catch(...)
    {
      return true;
    }

    ++cit;
    ++i;
  }

  error |= (i != mask.numberOfVoxels());
  error |= (64 != volumeBits.length());
  QString volumeTestValue("0101010101010101010101010101010101010101010101010101010101010101");
  error |= (volumeBits != volumeTestValue);

  return error;
}