/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// EspINA
#include "LargeFileHandler.hxx"

// ITK
#include <itkImage.h>
#include <itkImageFileReader.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace std;

int write_constructor( int argc, char** argv )
{
  int error = EXIT_SUCCESS;

  using RealVectorImageType = itk::VectorImage<float, 3>;
  using NormalImageType     = itk::Image<unsigned char, 3>;

  try
  {
    RealVectorImageType::SpacingType spacing;
    spacing[0] = 1;
    spacing[1] = 2;
    spacing[2] = 3;

    RealVectorImageType::IndexType index;
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;

    RealVectorImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

    RealVectorImageType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    auto dir = QDir::current();
    auto name = dir.absoluteFilePath("test");

    auto lfh = std::make_shared<LargeFileHandler<RealVectorImageType>>(name, region, spacing);
    auto lfhspacing = lfh->spacing();
    auto lfhRegion = lfh->region();
    auto lfhIndex = lfhRegion.GetIndex();
    auto lfhSize = lfhRegion.GetSize();
    auto lfhOrigin = lfh->origin();

    cout << "spacing " << lfhspacing[0] << " " << lfhspacing[1] << " " << lfhspacing[2] << endl;
    cout << "index " << lfhIndex[0] << " " << lfhIndex[1] << " " << lfhIndex[2] << endl;
    cout << "size " << lfhSize[0] << " " << lfhSize[1] << " " << lfhSize[2] << endl;
    cout << "origin " << lfhOrigin[0] << " " << lfhOrigin[1] << " " << lfhOrigin[2] << endl;
    lfhRegion.Print(std::cout);
  }
  catch(itk::ExceptionObject &excp)
  {
    cout << "EXCEPCION-> " << excp.GetDescription() << " " << excp.GetLocation() << std::endl << std::flush;
    error = EXIT_FAILURE;
  }

  return error;
}
