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

#include <itkImageRegionExclusionConstIteratorWithIndex.h>

using namespace EspINA;

using BMask = BinaryMask<unsigned char>;

int binaryMask_itkImage_dump(int argc, char** argv)
{
  Bounds bounds{ 0,4,0,4,0,4 };
  BMask *mask = new BMask(bounds);

  Bounds regionBounds{ 1,3,1,3,1,3 };
  BMask::region_iterator crit(mask, regionBounds);

  while (!crit.isAtEnd())
  {
    crit.Set();
    ++crit;
  }
  mask->setForegroundValue(1);

  itkVolumeType::Pointer image = mask->itkImage();
  //image->Print(std::cout);
  itkVolumeType::RegionType region = image->GetLargestPossibleRegion();
  region.Print(std::cout);
  itk::ImageRegionConstIteratorWithIndex<itkVolumeType> it(image, region);
  itkVolumeType::IndexType imageIndex;

  int count = 0;
  for(auto x = 0; x < 5; ++x)
  {
    for(auto y = 0; y < 5; ++y)
    {
      for(auto z = 0; z < 5; ++z)
      {
        imageIndex[0] = x;
        imageIndex[1] = y;
        imageIndex[2] = z;

        std::cout << (int)image->GetPixel(imageIndex);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  return true;
}
