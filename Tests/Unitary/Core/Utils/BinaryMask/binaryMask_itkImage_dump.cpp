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

int binaryMask_itkImage_dump(int argc, char** argv)
{
  Bounds bounds{ 0,4,0,4,0,4 };
  BMask *mask = new BMask(bounds);

  itkVolumeType::Pointer image = mask->itkImage();
  //image->Print(std::cout);
  itkVolumeType::RegionType region = image->GetLargestPossibleRegion();

  itk::ImageRegionConstIterator<itkVolumeType> it(image, region);

  int count = 0;
  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    std::cout << "." << (int)it.Get();
    ++it;
    ++count;
  }
  std::cout << std::endl << "count:" << count << std::endl;

  return true;
}
