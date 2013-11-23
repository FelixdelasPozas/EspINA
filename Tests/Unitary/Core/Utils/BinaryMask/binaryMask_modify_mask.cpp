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

#include <itkImageRegionConstIterator.h>

using namespace EspINA;

using BMask = BinaryMask<unsigned char>;

int binaryMask_modify_mask(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{ 0,4,0,4,0,4 };
  BMask *mask = new BMask(bounds);
  BMask::iterator cit(mask);
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
        error |= (mask->foregroundValue() != cit.Get());
        volumeBits += "1";
      }
      else
      {
        error |= (mask->backgroundValue() != cit.Get());
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

  error |= (i != mask->numberOfVoxels());
  error |= (64 != volumeBits.length());
  QString volumeTestValue("0101010101010101010101010101010101010101010101010101010101010101");
  error |= (volumeBits != volumeTestValue);

  return error;
}
