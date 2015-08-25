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
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds)
  {
    using itk2vtkImageFilter = itk::ImageToVTKImageFilter<T>;
    using ExtractFilter = itk::ExtractImageFilter<T,T>;

    typename T::Pointer itkImage;

    auto spacing = volume->GetSpacing();
    auto origin = volume->GetOrigin();
    VolumeBounds iBounds(inputBounds, NmVector3{spacing[0], spacing[1], spacing[2]}, NmVector3{origin[0], origin[1], origin[2]});

    auto vBounds = volumeBounds<T>(volume, volume->GetLargestPossibleRegion());
    // check if the requested bounds are inside the volume bounds, else fail miserably
    Q_ASSERT(contains(vBounds, iBounds));

    if (!isEquivalent(vBounds, iBounds))
    {
      auto extractor = ExtractFilter::New();
      extractor->SetExtractionRegion(equivalentRegion<T>(volume, inputBounds));
      extractor->SetInput(volume);
      extractor->Update();

      itkImage = extractor->GetOutput();
    }
    else
    {
      itkImage = volume;
    }

    itkImage->DisconnectPipeline();

    auto transform = itk2vtkImageFilter::New();
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

  //-----------------------------------------------------------------------------
  template<typename T>
  bool isSegmentationVoxel(const Output::ReadLockData<VolumetricData<T>> &volume, const NmVector3 &point)
  {
    Bounds bounds{point};

    bool result = contains(volume->bounds(), bounds, volume->spacing());

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
      drawingBounds = equivalentBounds<T>(drawnVolume, drawnVolume->GetLargestPossibleRegion());
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
      drawingBounds = equivalentBounds<T>(drawnVolume, drawnVolume->GetLargestPossibleRegion());
    }

    volume->resize(boundingBox(drawingBounds, volume->bounds()));
    volume->draw(drawnVolume);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, const BinaryMaskSPtr<unsigned char> &mask)
  {
    volume->resize(boundingBox(mask->bounds().bounds(), volume->bounds()));
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
    auto it     = itk::ImageRegionIterator<T>(image, region);

    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionIteratorWithIndex<T> itkImageIteratorWithIndex(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    auto it     = itk::ImageRegionIteratorWithIndex<T>(image, region);

    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionConstIterator<T> itkImageConstIterator(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    auto it     = itk::ImageRegionConstIterator<T>(image, region);

    it.GoToBegin();

    return it;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  itk::ImageRegionConstIteratorWithIndex<T> itkImageConstIteratorWithIndex(typename T::Pointer image, const Bounds &bounds)
  {
    auto region = equivalentRegion<T>(image, bounds);
    auto it     =  itk::ImageRegionConstIteratorWithIndex<T>(image, region);

    it.GoToBegin();

    return it;
  }
} // namespace ESPINA
