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

      if(region != equivalentRegion<itkVolumeType>(NmVector3{0,0,0}, file_W->bounds().spacing(), file_W->bounds().bounds()))
      {
        std::cout << "write constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != file_W->itkRegion())
      {
        std::cout << "write constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      itkVolumeType::PointType W_origin;
      W_origin[0] = 0;
      W_origin[1] = 2.2;
      W_origin[2] = 6.6;
      if(file_W->itkOriginalOrigin() != W_origin)
      {
        std::cout << "write constructor -> invalid itk origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if (file_W->bounds().origin() != NmVector3{0, 0, 0})
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

      if(region != equivalentRegion<itkVolumeType>(NmVector3{0,0,0}, file_R->bounds().spacing(), file_R->bounds().bounds()))
      {
        std::cout << "read constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != file_R->itkRegion())
      {
        std::cout << "read constructor -> invalid itkregion(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      itkVolumeType::PointType R_origin;
      R_origin[0] = 0;
      R_origin[1] = 2.2;
      R_origin[2] = 6.6;
      if (file_R->itkOriginalOrigin() != R_origin)
      {
        std::cout << "read constructor -> invalid itk origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if (file_R->bounds().origin() != NmVector3{0, 0, 0})
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
      auto file_W = std::make_shared<WritableStreamedVectorVolume<itkVolumeType_2>>(filename, region, spacing, 2);

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

      if(origin != file_W->itkOriginalOrigin())
      {
        std::cout << "write constructor -> invalid itkOrigin(). line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(2 != file_W->vectorLength())
      {
        std::cout << "write constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      auto file_R = std::make_shared<StreamedVolume<itkVolumeType_2>>(filename);

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

      if(origin != file_R->itkOriginalOrigin())
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

  // --- itk::image float dims=3 size=3
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
      auto file_W = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(filename, region, spacing, 3);

      if(NmVector3{1.1,2.2,3.3} != file_W->bounds().spacing())
      {
        std::cout << "write constructor -> invalid spacing. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != equivalentRegion<RealVectorImageType>(NmVector3{0,0,0}, file_W->bounds().spacing(), file_W->bounds().bounds()))
      {
        std::cout << "write constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      RealVectorImageType::PointType W_origin;
      W_origin[0] = 0;
      W_origin[1] = 2.2;
      W_origin[2] = 6.6;
      if(W_origin != file_W->itkOriginalOrigin())
      {
        std::cout << "write constructor -> invalid itk origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(NmVector3{0,0,0} != file_W->bounds().origin())
      {
        std::cout << "write constructor -> invalid origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(3 != file_W->vectorLength())
      {
        std::cout << "write constructor -> invalid vector size. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      auto file_R = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(filename, region, spacing, 3);

      if(NmVector3{1.1,2.2,3.3} != file_R->bounds().spacing())
      {
        std::cout << "read constructor -> invalid spacing. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(region != equivalentRegion<RealVectorImageType>(NmVector3{0,0,0}, file_R->bounds().spacing(), file_R->bounds().bounds()))
      {
        std::cout << "read constructor -> invalid region. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      RealVectorImageType::PointType R_origin;
      R_origin[0] = 0;
      R_origin[1] = 2.2;
      R_origin[2] = 6.6;
      if(R_origin != file_R->itkOriginalOrigin())
      {
        std::cout << "read constructor -> invalid itk origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(NmVector3{0,0,0} != file_R->bounds().origin())
      {
        std::cout << "read constructor -> invalid origin. line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
      }

      if(3 != file_R->vectorLength())
      {
        std::cout << "read constructor -> invalid vector size. line " << __LINE__ << std::endl;
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
      auto file = std::make_shared<StreamedVolume<RealVectorImageType>>(QFileInfo{"idontexists.mhd"});
      std::cout << "read constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the file doesn't exist.
    }

    try
    {
      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType>>(QFileInfo{"idontexists.mhd"}, RealVectorImageType::RegionType(), RealVectorImageType::SpacingType());
      std::cout << "Write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the parameters are invalid
    }

    try
    {
      auto file = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(QFileInfo{"idontexists.mhd"}, RealVectorImageType::RegionType(), RealVectorImageType::SpacingType());
      std::cout << "Vector write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the parameters are invalid
    }

    try
    {
      RealVectorImageType::SpacingType spacing;
      spacing[0] = 1.1;
      spacing[1] = 2.2;
      spacing[2] = 3.3;

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

      auto file = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(QFileInfo{"shouldfail.mhd"}, region, spacing, 0);
      std::cout << "Vector write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because the parameters are invalid
    }
  }

  {
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

      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType>>(filename, RealVectorImageType::RegionType(), spacing);
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

      auto file = std::make_shared<WritableStreamedVolume<RealVectorImageType>>(filename, region, RealVectorImageType::SpacingType());
      std::cout << "write constructor -> should throw exception. line " << __LINE__ << std::endl;
      error = EXIT_FAILURE;
    }
    catch(...)
    {
      // an exception should be thrown because invalid spacing
    }
  }

  QFile::remove(filename);
  auto filename2 = dir.absoluteFilePath("test.raw");
  QFile::remove(filename2);

  std::cout << "exit value: " << error << std::endl;

  return error;
}
