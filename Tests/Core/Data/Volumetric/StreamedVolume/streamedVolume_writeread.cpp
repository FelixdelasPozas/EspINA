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

#include "Core/Analysis/Data/Volumetric/StreamedVolume.hxx"
#include "Core/Analysis/Data/Volumetric/WritableStreamedVolume.hxx"

#include <itkImageRegionIterator.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace std;

int streamedVolume_writeread( int argc, char** argv )
{
  int error = EXIT_SUCCESS;

  auto dir      = QDir::current();
  auto filename = dir.absoluteFilePath("test.mhd");
  auto info     = QFileInfo(filename);

  if(info.exists())
  {
    QFile::remove(info.absoluteFilePath());
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);
  }

  using RealVectorImageType = itk::VectorImage<float, 3>;

  try
  {
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
    size[2] = 100;

    RealVectorImageType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    auto creator = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(filename, region, spacing, 3);

    auto bounds  = creator->bounds();
    for(int i = vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]); i < vtkMath::Round((bounds.bounds()[5]+(spacing[2]/2))/spacing[2]); ++i)
    {
      auto sliceBounds = bounds.bounds();
      sliceBounds[4] = sliceBounds[5] = i*spacing[2];

      auto sliceImage = creator->itkImage(sliceBounds);
      {
        auto region = sliceImage->GetLargestPossibleRegion();
        auto pixel = sliceImage->GetPixel(region.GetIndex());
        if(creator->vectorLength() != pixel.GetNumberOfElements())
        {
          std::cout << "invalid vector size (" << pixel.GetNumberOfElements() << ") image is (" << creator->vectorLength() << ") line " << __LINE__ << std::endl;
          error = EXIT_FAILURE;
          break;
        }
      }

      itk::ImageRegionIterator<RealVectorImageType> it(sliceImage, sliceImage->GetLargestPossibleRegion());
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        RealVectorImageType::ValueType value;
        value.SetSize(3);
        value.SetElement(0, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]));
        value.SetElement(1, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]) + 1);
        value.SetElement(2, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]) + 2);

        it.Set(value);
        ++it;
      }

      creator->draw(sliceImage);
    }
  }
  catch(const EspinaException &excp)
  {
    qDebug() << "exception:" << excp.what();
    qDebug() << "details:" << excp.details();
    error = EXIT_FAILURE;
  }
  catch(const itk::ExceptionObject &excp)
  {
    qDebug() << "exception:" << QString(excp.what());
    qDebug() << "details:" << QString(excp.GetDescription());
    qDebug() << "file:" << QString(excp.GetFile());
    qDebug() << "location:" << QString(excp.GetLocation());
    error = EXIT_FAILURE;
  }

  try
  {
    info.refresh(); // else returns false on exists(), why? catching is enabled...

    auto file = std::make_shared<StreamedVolume<RealVectorImageType>>(info);

    auto bounds  = file->bounds();
    auto spacing = file->bounds().spacing();

    for(int i = vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]); i < vtkMath::Round((bounds.bounds()[5]+(spacing[2]/2))/spacing[2]); ++i)
    {
      auto sliceBounds = bounds.bounds();
      sliceBounds[4] = sliceBounds[5] = i*spacing[2];

      auto sliceImage = file->itkImage(sliceBounds);

      auto region = sliceImage->GetLargestPossibleRegion();
      auto pixel = sliceImage->GetPixel(region.GetIndex());
      if(file->vectorLength() != pixel.GetNumberOfElements())
      {
        std::cout << "invalid vector size " << pixel.GetNumberOfElements() << " image is " << file->vectorLength() << std::endl;
        error = EXIT_FAILURE;
        return error;
      }

      RealVectorImageType::ValueType test;
      test.SetSize(3);
      test.SetElement(0, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]));
      test.SetElement(1, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]) + 1);
      test.SetElement(2, i - vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]) + 2);

      itk::ImageRegionIterator<RealVectorImageType> it(sliceImage, sliceImage->GetLargestPossibleRegion());
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        RealVectorImageType::ValueType value = it.Get();
        if((value.GetElement(0) != test.GetElement(0)) || (value.GetElement(1) != test.GetElement(1)) || (value.GetElement(2) != test.GetElement(2)))
        {
          std::cout << "invalid vector value [" << value.GetElement(0) << "," << value.GetElement(1) << "," << value.GetElement(2) << "] != [" << test.GetElement(0) << "," << test.GetElement(1) << "," << test.GetElement(2) << "]" << std::endl;
          error = EXIT_FAILURE;
          return error;
        }

        ++it;
      }
    }
  }
  catch(const EspinaException &excp)
  {
    qDebug() << "exception:" << excp.what();
    qDebug() << "details:" << excp.details();
    error = EXIT_FAILURE;
  }
  catch(const itk::ExceptionObject &excp)
  {
    qDebug() << "exception:" << QString(excp.what());
    qDebug() << "details:" << QString(excp.GetDescription());
    qDebug() << "file:" << QString(excp.GetFile());
    qDebug() << "location:" << QString(excp.GetLocation());
    error = EXIT_FAILURE;
  }

  QFile::remove(info.absoluteFilePath());
  auto filename2 = dir.absoluteFilePath("test.raw");
  QFile::remove(filename2);

  try
  {
    using RealImageType = itk::Image<double, 3>;

    RealImageType::SpacingType spacing;
    spacing[0] = 1.1;
    spacing[1] = 2.2;
    spacing[2] = 3.3;

    RealImageType::IndexType index;
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;

    RealImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

    RealImageType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    auto creator = std::make_shared<WritableStreamedVolume<RealImageType>>(filename, region, spacing);

    auto bounds  = creator->bounds();
    for(int i = vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]); i < vtkMath::Round((bounds.bounds()[5]+(spacing[2]/2))/spacing[2]); ++i)
    {
      auto sliceBounds = bounds.bounds();
      sliceBounds[4] = sliceBounds[5] = i*spacing[2];

      auto sliceImage = creator->itkImage(sliceBounds);
      {
        auto region = sliceImage->GetLargestPossibleRegion();
        auto pixel = sliceImage->GetPixel(region.GetIndex());
        if(creator->vectorLength() != 1)
        {
          std::cout << "invalid image vector size " << creator->vectorLength() << ". line " << __LINE__ << std::endl;
          error = EXIT_FAILURE;
          break;
        }

        if(pixel != 0.0)
        {
          std::cout << "invalid pixel value after creation " << pixel << "should be 0. line " << __LINE__ << std::endl;
          error = EXIT_FAILURE;
          break;
        }
      }

      itk::ImageRegionIterator<RealImageType> it(sliceImage, sliceImage->GetLargestPossibleRegion());
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        RealImageType::ValueType value = 0;
        auto index = it.GetIndex();

        for(unsigned int j = 0; j < RealImageType::GetImageDimension(); ++j)
        {
          value += index.GetElement(j);
        }

        it.Set(value);
        ++it;
      }

      creator->draw(sliceImage);
    }
  }
  catch(const EspinaException &excp)
  {
    qDebug() << "exception:" << excp.what();
    qDebug() << "details:" << excp.details();
    error = EXIT_FAILURE;
  }
  catch(const itk::ExceptionObject &excp)
  {
    qDebug() << "exception:" << QString(excp.what());
    qDebug() << "details:" << QString(excp.GetDescription());
    qDebug() << "file:" << QString(excp.GetFile());
    qDebug() << "location:" << QString(excp.GetLocation());
    error = EXIT_FAILURE;
  }

  try
  {
    info.refresh(); // else returns false on exists(), why? catching is enabled...

    using RealImageType = itk::Image<double, 3>;

    auto file = std::make_shared<StreamedVolume<RealImageType>>(info);

    auto bounds  = file->bounds();
    auto spacing = file->bounds().spacing();

    for(int i = vtkMath::Round((bounds.bounds()[4]+(spacing[2]/2))/spacing[2]); i < vtkMath::Round((bounds.bounds()[5]+(spacing[2]/2))/spacing[2]); ++i)
    {
      auto sliceBounds = bounds.bounds();
      sliceBounds[4] = sliceBounds[5] = i*spacing[2];

      auto sliceImage = file->itkImage(sliceBounds);

      if(file->vectorLength() != 1)
      {
        std::cout << "invalid image vector size " << file->vectorLength() << ". line " << __LINE__ << std::endl;
        error = EXIT_FAILURE;
        return error;
      }

      itk::ImageRegionIterator<RealImageType> it(sliceImage, sliceImage->GetLargestPossibleRegion());
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        RealImageType::ValueType test = 0;
        auto index = it.GetIndex();
        for(unsigned int j = 0; j < RealImageType::GetImageDimension(); ++j)
        {
          test += index.GetElement(j);
        }

        if(it.Get() != test)
        {
          std::cout << "invalid non vector value [" << it.Get() << "] != [" << test << "]" << std::endl;
          error = EXIT_FAILURE;
          return error;
        }

        ++it;
      }
    }
  }
  catch(const EspinaException &excp)
  {
    qDebug() << "exception:" << excp.what();
    qDebug() << "details:" << excp.details();
    error = EXIT_FAILURE;
  }
  catch(const itk::ExceptionObject &excp)
  {
    qDebug() << "exception:" << QString(excp.what());
    qDebug() << "details:" << QString(excp.GetDescription());
    qDebug() << "file:" << QString(excp.GetFile());
    qDebug() << "location:" << QString(excp.GetLocation());
    error = EXIT_FAILURE;
  }

  QFile::remove(info.absoluteFilePath());
  QFile::remove(filename2);

  std::cout << "exit value: " << error << std::endl;

  return error;
}
