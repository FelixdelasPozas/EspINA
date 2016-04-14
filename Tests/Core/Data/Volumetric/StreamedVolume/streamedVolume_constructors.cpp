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
#include <Core/Utils/SpatialUtils.hxx>
#include "Core/Analysis/Data/Volumetric/StreamedVolume.hxx"
#include "Core/Analysis/Data/Volumetric/WritableStreamedVolume.hxx"

// ITK
#include <itkImage.h>
#include <itkImageFileReader.h>

#include <typeinfo>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace std;

int streamedVolume_constructors( int argc, char** argv )
{
  int error     = EXIT_SUCCESS;
  auto dir      = QDir::current();
  auto filename = dir.absoluteFilePath("test.mhd");
  auto info     = QFileInfo(filename);

  if(info.exists())
  {
    QFile::remove(filename);
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);
  }

  // Test different dimensions and vector sizes.
  // itk::image unsigned char dims=3 size=1
  {
    itkVolumeType::SpacingType spacing;
    spacing[0] = 1.1;
    spacing[1] = 2.2;
    spacing[2] = 3.3;

    itkVolumeType::IndexType index;
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;

    itkVolumeType::SizeType size;
    size[0] = 1000;
    size[1] = 100;
    size[2] = 10;

    itkVolumeType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    try
    {
      auto file_W = std::make_shared<WritableStreamedVolume<itkVolumeType>>(filename, region, spacing);

      if(NmVector3{1.1,2.2,3.3} != file_W->bounds().spacing())
      {
        std::cout << "write constructor -> invalid spacing. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != equivalentRegion<itkVolumeType>(file_W->bounds().origin(), file_W->bounds().spacing(), file_W->bounds().bounds()))
      {
        std::cout << "write constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != file_W->itkRegion())
      {
        std::cout << "write constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if (file_W->bounds().origin() != NmVector3{0, 2.2, 6.6})
      {
        std::cout << "write constructor -> invalid origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(1 != file_W->vectorLength())
      {
        std::cout << "write constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      auto file_R = std::make_shared<StreamedVolume<itkVolumeType>>(filename);

      if(NmVector3{1.1,2.2,3.3} != file_R->bounds().spacing())
      {
        std::cout << "read constructor -> invalid spacing. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != equivalentRegion<itkVolumeType>(file_R->bounds().origin(), file_R->bounds().spacing(), file_R->bounds().bounds()))
      {
        std::cout << "read constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != file_R->itkRegion())
      {
        std::cout << "read constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if (file_R->bounds().origin() != NmVector3{0, 2.2, 6.6})
      {
        std::cout << "read constructor -> invalid origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(1 != file_R->vectorLength())
      {
        std::cout << "read constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }
    }
    catch(const EspinaException &excp)
    {
      std::cout << "exception 1. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << excp.what();
      qDebug() << "details:"   << excp.details();
      error = EXIT_FAILURE;
    }
    catch(const itk::ExceptionObject &excp)
    {
      std::cout << "exception 1. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << QString(excp.what());
      qDebug() << "details:"   << QString(excp.GetDescription());
      qDebug() << "file:"      << QString(excp.GetFile());
      qDebug() << "location:"  << QString(excp.GetLocation());
      error = EXIT_FAILURE;
    }
  }

  // --- itk::image unsigned char dims=2 size=2
  {
    QFile::remove(filename);
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);

    using itkVolumeType_2 = itk::VectorImage<unsigned char, 2>;

    itkVolumeType_2::SpacingType spacing;
    spacing[0] = 1.1;
    spacing[1] = 2.2;

    itkVolumeType_2::IndexType index;
    index[0] = 0;
    index[1] = 1;

    itkVolumeType_2::PointType origin;
    origin[0] = index[0]*spacing[0];
    origin[1] = index[1]*spacing[1];

    itkVolumeType_2::SizeType size;
    size[0] = 1000;
    size[1] = 100;

    itkVolumeType_2::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    try
    {
      auto file_W = std::make_shared<WritableStreamedVolume<itkVolumeType_2, 2>>(filename, region, spacing);

      if(region != file_W->itkRegion())
      {
        std::cout << "write constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(spacing != file_W->itkSpacing())
      {
        std::cout << "write constructor -> invalid itkSpacing(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(origin != file_W->itkOrigin())
      {
        std::cout << "write constructor -> invalid itkOrigin(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(2 != file_W->vectorLength())
      {
        std::cout << "write constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      auto file_R = std::make_shared<StreamedVolume<itkVolumeType_2, 2>>(filename);

      if(region != file_R->itkRegion())
      {
        std::cout << "read constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(spacing != file_R->itkSpacing())
      {
        std::cout << "read constructor -> invalid itkSpacing(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(origin != file_R->itkOrigin())
      {
        std::cout << "read constructor -> invalid itkOrigin(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(2 != file_R->vectorLength())
      {
        std::cout << "read constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }
    }
    catch(const EspinaException &excp)
    {
      std::cout << "exception 2. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << excp.what();
      qDebug() << "details:"   << excp.details();
      error = EXIT_FAILURE;
    }
    catch(const itk::ExceptionObject &excp)
    {
      std::cout << "exception 2. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << QString(excp.what());
      qDebug() << "details:"   << QString(excp.GetDescription());
      qDebug() << "file:"      << QString(excp.GetFile());
      qDebug() << "location:"  << QString(excp.GetLocation());
      error = EXIT_FAILURE;
    }
  }

  using RealVectorImageType = itk::VectorImage<float, 3>;
  {
    QFile::remove(filename);
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);

    RealVectorImageType::SpacingType spacing;
    spacing[0] = 1.1;
    spacing[1] = 2.2;
    spacing[2] = 3.3;

    RealVectorImageType::IndexType index;
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;

    RealVectorImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 10;

    RealVectorImageType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    try
    {
      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType,3>>(filename, region, spacing);

      if(NmVector3{1.1,2.2,3.3} != file->bounds().spacing())
      {
        std::cout << "write constructor -> invalid spacing. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != equivalentRegion<RealVectorImageType>(file->bounds().origin(), file->bounds().spacing(), file->bounds().bounds()))
      {
        std::cout << "write constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(NmVector3{0,2.2, 6.6} != file->bounds().origin())
      {
        std::cout << "write constructor -> invalid origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(3 != file->vectorLength())
      {
        std::cout << "write constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }
    }
    catch(const EspinaException &excp)
    {
      std::cout << "exception 3. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << excp.what();
      qDebug() << "details:"   << excp.details();
      error = EXIT_FAILURE;
    }
    catch(const itk::ExceptionObject &excp)
    {
      std::cout << "exception 3. line " << __LINE__ << std::endl;
      qDebug() << "exception:" << QString(excp.what());
      qDebug() << "details:"   << QString(excp.GetDescription());
      qDebug() << "file:"      << QString(excp.GetFile());
      qDebug() << "location:"  << QString(excp.GetLocation());
      error = EXIT_FAILURE;
    }
  }

  {
    try
    {
      RealVectorImageType::SizeType size;
      size[0] = 1;
      size[1] = 1;
      size[2] = 2;

      RealVectorImageType::IndexType index;
      index[0] = 0;
      index[1] = 0;
      index[2] = 0;

      RealVectorImageType::RegionType region;
      region.SetIndex(index);
      region.SetSize(size);

      auto file = std::make_shared<StreamedVolume<RealVectorImageType,2>>(filename);
      std::cout << "read constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the file already exists but vector size is incorrect.
    }

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
      size[2] = 10;

      RealVectorImageType::RegionType region;
      region.SetIndex(index);
      region.SetSize(size);

      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType,2>>(filename, region, spacing);
      std::cout << "write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the file already exists but the vector size is incorrect.
    }

    QFile::remove(filename);
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);

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
      size[2] = 10;

      RealVectorImageType::RegionType region;
      region.SetIndex(index);
      region.SetSize(size);

      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType,3>>(filename, RealVectorImageType::RegionType(), spacing);
      std::cout << "write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because invalid regiontype
    }

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
      size[2] = 10;

      RealVectorImageType::RegionType region;
      region.SetIndex(index);
      region.SetSize(size);

      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType,3>>(filename, region, RealVectorImageType::SpacingType());
      std::cout << "write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because invalid spacing
    }
  }

  std::cout << "exit value: " << error << std::endl;

  return error;
}
