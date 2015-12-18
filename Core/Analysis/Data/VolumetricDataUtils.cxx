/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/VolumeBounds.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Analysis/Output.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  template<typename T>
  bool isSegmentationVoxel(const Output::ReadLockData<VolumetricData<T>> &volume, const NmVector3 &point)
  {
    Bounds bounds{point};

    bool result = contains(volume->bounds(), bounds, volume->bounds().spacing());

    if (result)
    {
      typename T::Pointer voxel = volume->itkImage(bounds);

      result = (SEG_BG_VALUE != *(static_cast<unsigned char*>(voxel->GetBufferPointer())));
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, const Bounds &bounds)
  {
    expandAndDraw<T>(volume.get(), drawnVolume, bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void expandAndDraw(VolumetricData<T> *volume, typename T::Pointer drawnVolume, const Bounds &bounds)
  {
    Bounds drawingBounds = bounds;

    if (!drawingBounds.areValid())
    {
      drawingBounds = equivalentBounds<T>(drawnVolume);
    }

    volume->resize(boundingBox(drawingBounds, volume->bounds()));
    volume->draw(drawnVolume);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, typename T::Pointer drawnVolume, const Bounds &bounds)
  {
    Bounds drawingBounds = bounds;

    if (!drawingBounds.areValid())
    {
      drawingBounds = equivalentBounds<T>(drawnVolume);
    }

    volume->resize(boundingBox(drawingBounds, volume->bounds()));
    volume->draw(drawnVolume);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, const BinaryMaskSPtr<unsigned char> &mask)
  {
    volume->resize(boundingBox(mask->bounds(), volume->bounds()));
    volume->draw(mask, mask->foregroundValue());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void fitToContents(Output::WriteLockData<VolumetricData<T>> &volume, typename T::ValueType bgValue)
  {
    auto bounds = minimalBounds<T>(volume->itkImage(), bgValue);
    volume->resize(bounds);
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                bounds,
                                      const typename T::ValueType  value,
                                      const NmVector3             &spacing,
                                      const NmVector3             &origin)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    image->SetRegions(equivalentRegion<T>(image, bounds));
    image->Allocate();
    image->FillBuffer(value);

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void exportVolume(typename T::Pointer volume, const QString &path)
  {
    bool releaseFlag = volume->GetReleaseDataFlag();
    volume->ReleaseDataFlagOff();

    auto writer = itk::ImageFileWriter<T>::New();
    writer->SetFileName(path.toUtf8().data());
    writer->SetInput(volume);
    writer->Write();
    volume->SetReleaseDataFlag(releaseFlag);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot createSnapshot(typename T::Pointer   volume,
                          TemporalStorageSPtr   storage,
                          const QString        &path,
                          const QString        &id)
  {
    Snapshot snapshot;

    storage->makePath(path);

    QString mhd = path + id;
    QString raw = mhd;
    raw.replace(".mhd",".raw");

    exportVolume<T>(volume, storage->absoluteFilePath(mhd));

    snapshot << SnapshotData(mhd, storage->snapshot(mhd));
    snapshot << SnapshotData(raw, storage->snapshot(raw));

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer readVolume(const QString &filename)
  {
    using VolumeReader = itk::ImageFileReader<T>;

    typename VolumeReader::Pointer reader = VolumeReader::New();
    reader->SetFileName(filename.toUtf8().data());
    reader->Update();

    auto image = reader->GetOutput();

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionIterator<T> itkImageIterator(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    if(!image->GetLargestPossibleRegion().IsInside(region))
    {
      region.Crop(image->GetLargestPossibleRegion());
      qWarning() << "itkImageRegionIterator<T>(image,bounds) -> asked for a region partially outside the image bounds!";
    }

    auto it = itk::ImageRegionIterator<T>(image, region);
    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionIteratorWithIndex<T> itkImageIteratorWithIndex(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    if(!image->GetLargestPossibleRegion().IsInside(region))
    {
      region.Crop(image->GetLargestPossibleRegion());
      qWarning() << "itkImageRegionIteratorWithIndex<T>(image,bounds) -> asked for a region partially outside the image bounds!";
    }

    auto it = itk::ImageRegionIteratorWithIndex<T>(image, region);
    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionConstIterator<T> itkImageConstIterator(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    if(!image->GetLargestPossibleRegion().IsInside(region))
    {
      region.Crop(image->GetLargestPossibleRegion());
      qWarning() << "itkImageRegionConstIterator<T>(image,bounds) -> asked for a region partially outside the image bounds!";
    }

    auto it = itk::ImageRegionConstIterator<T>(image, region);
    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionConstIteratorWithIndex<T> itkImageConstIteratorWithIndex(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    if(!image->GetLargestPossibleRegion().IsInside(region))
    {
      region.Crop(image->GetLargestPossibleRegion());
      qWarning() << "itkImageRegionConstIteratorWithIndex<T>(image,bounds) -> asked for a region partially outside the image bounds!";
    }

    auto it =  itk::ImageRegionConstIteratorWithIndex<T>(image, region);
    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void changeSpacing(typename T::Pointer image, typename T::SpacingType &spacing)
  {
    auto imageSpacing = image->GetSpacing();

    NmVector3 ratio{
      spacing[0]/imageSpacing[0],
      spacing[1]/imageSpacing[1],
      spacing[2]/imageSpacing[2]
    };

    changeSpacing<T>(image, spacing, ratio);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void changeSpacing(typename T::Pointer image, typename T::SpacingType &spacing, const NmVector3 &ratio)
  {
    auto origin = image->GetOrigin();

    for (int i = 0; i < 3; ++i)
    {
      origin[i] *= ratio[i];
    }

    //auto preBlockBounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
    image->SetOrigin(origin);
    image->SetSpacing(spacing);
    image->Update();
    //auto postBlockBounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
    //qDebug() << "Update image bounds" << preBlockBounds << "to"<< postBlockBounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer extract_image(typename T::Pointer const sourceImage, const typename T::RegionType &region)
  {
    auto sourceRegion = sourceImage->GetLargestPossibleRegion();
    auto componentsNum = sourceImage->GetNumberOfComponentsPerPixel();

    if(!sourceRegion.IsInside(region))
    {
      auto what = QObject::tr("Attempt to extract a region outside of the source image region.");
      auto details = QObject::tr("extract_image(image, region) ->") + what;

      throw Core::Utils::EspinaException(what, details);
    }

    typename T::Pointer image = T::New();
    image->SetSpacing(sourceImage->GetSpacing());
    image->SetOrigin(sourceImage->GetOrigin());
    image->SetRegions(region);
    image->SetNumberOfComponentsPerPixel(componentsNum);
    image->SetReleaseDataFlag(false);
    image->Allocate();

    auto thisPointer = sourceImage->GetBufferPointer();
    auto otherPointer = image->GetBufferPointer();

    auto zJump     = sourceRegion.GetSize(0)*sourceRegion.GetSize(1)*componentsNum;
    auto yJump     = sourceRegion.GetSize(0)*componentsNum;
    auto zStart    = region.GetIndex(2);
    auto zEnd      = zStart+region.GetSize(2);
    auto yStart    = region.GetIndex(1);
    auto yEnd      = yStart+region.GetSize(1);
    auto copySize  = region.GetSize(0)*componentsNum;
    auto copyStart = region.GetIndex(0)*componentsNum;

    for(auto z = zStart; z < zEnd; ++z)
    {
      for(auto y = yStart; y < yEnd; ++y)
      {
        auto pointer = thisPointer + z*zJump + y*yJump + copyStart;
        std::memcpy(otherPointer, pointer, copySize);
        otherPointer += copySize;
      }
    }

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer extract_image(typename T::Pointer const sourceImage, const VolumeBounds &bounds)
  {
    auto region = equivalentRegion<T>(sourceImage, bounds);

    return extract_image<T>(sourceImage, region);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds)
  {
    using itk2vtkImageFilter = itk::ImageToVTKImageFilter<T>;

    typename T::Pointer itkImage;

    auto spacing = volume->GetSpacing();
    auto origin = volume->GetOrigin();
    VolumeBounds iBounds(inputBounds, NmVector3{spacing[0], spacing[1], spacing[2]}, NmVector3{origin[0], origin[1], origin[2]});

    auto vBounds = volumeBounds<T>(volume, volume->GetLargestPossibleRegion());

    if(!contains(vBounds, iBounds))
    {
      auto what    = QObject::tr("The extraction region %1 is not completely contained in image region %2").arg(iBounds.toString()).arg(vBounds.toString());
      auto details = QObject::tr("vtkImage(image, bounds) ->") + what;

      throw Core::Utils::EspinaException(what, details);
    }

    if (!isEquivalent(vBounds, iBounds))
    {
      itkImage = extract_image<T>(volume, iBounds);
    }
    else
    {
      itkImage = volume;
      itkImage->DisconnectPipeline();
    }

    auto transform = itk2vtkImageFilter::New();
    transform->SetNumberOfThreads(1);
    transform->SetInput(itkImage);
    transform->Update();

    auto returnImage = vtkSmartPointer<vtkImageData>::New();
    returnImage->DeepCopy(transform->GetOutput());
    return returnImage;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const Output::ReadLockData<VolumetricData<T>> &volume, const Bounds &bounds)
  {
    typename T::Pointer image = volume->itkImage(bounds);

    return vtkImage<T>(image, bounds);
  }

} // namespace ESPINA
