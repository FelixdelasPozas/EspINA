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

#ifndef ESPINA_VOLUMETRIC_DATA_UTILS_H
#define ESPINA_VOLUMETRIC_DATA_UTILS_H

// ESPINA
#include "VolumetricData.hxx"
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/VolumeBounds.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Analysis/Output.h>

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

// ITK
#include <itkExtractImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace ESPINA
{
  /** \brief Return the vtkImageData of specified bounds equivalent to the itkImage.
   * \param[in] volume itk image smart pointer to transform.
   * \param[in] inputBounds bounds of the image to transform.
   *
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds);

  /** \brief Return the vtkImageData of specified bounds equivalent to the volumetric data.
   * \param[in] volume VolumetricData smart pointer to transform.
   * \param[in] bounds bounds of the image to transform.
   *
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const Output::ReadLockData<VolumetricData<T>> &volume, const Bounds &bounds);

  /** \brief Volume's voxel's index at given spatial position.
   * \param[in] x x coordinate.
   * \param[in] y y coordinate.
   * \param[in] z z coordinate.
   *
   *  It doesn't check whether the index is valid or not
   */
  template<typename T>
  typename T::IndexType index(Nm x, Nm y, Nm z);

  /** \brief Returns whether or not the voxel at @point is not background.
   * \param[in] volume VolumetricData smart pointer.
   * \param[in] point point to check.
   *
   */
  template<typename T>
  bool isSegmentationVoxel(const VolumetricDataSPtr<T> volume, const NmVector3 &point);

  /** \brief Draw @drawnVolume into @volume, resizing @volume bounds to fit @drawnVolume if necessary.
   * \param[in] volume VolumetricData smart pointer to expand and draw.
   * \param[in] drawnVolume itk image smart pointer to draw into @volume.
   * \param[in] bounds bounds object to add if necessary.
   *
   */
  template<typename T>
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, const Bounds &bounds = Bounds());

  /** \brief Draw @drawnVolume into @volume, resizing @volume bounds to fit @drawnVolume if necessary.
   * \param[in] volume VolumetricData pointer to expand and draw.
   * \param[in] drawnVolume itk image pointer to draw into @volume.
   * \param[in] bounds bounds object to add if necessary.
   *
   */
  template<typename T>
  void expandAndDraw(VolumetricData<T> *volume, typename T::Pointer drawnVolume, const Bounds &bounds = Bounds());

  template<typename T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, typename T::Pointer drawnVolume, const Bounds &bounds = Bounds());

  template<class T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, const BinaryMaskSPtr<unsigned char> &mask);

  /** \brief Resizes the image to the minimum bounds that can contain the volume.
   * \param[in] volume volume to transform.
   * \param[in] bgValue background value of the image.
   *
   *  The resultant image is always smaller or equal in size to the original one.
   */
  template<typename T>
  inline void fitToContents(VolumetricDataSPtr<T> volume, typename T::ValueType bgValue)
  {
    auto bounds = minimalBounds<T>(volume->itkImage(), bgValue);
    volume->resize(bounds);
  }

  /** \brief Returns a new itk image smart pointer of the given spacing, origin and bounds, filled with @value.
   * \param[in] bounds bounds of the resultant image.
   * \param[in] value value to fill the image.
   * \param[in] spacing spacing of the resultant image.
   * \param[in] origin origin of the resultant image.
   *
   */
  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                 bounds,
                                      const typename T::ValueType   value   = 0,
                                      const NmVector3              &spacing = {1, 1, 1},
                                      const NmVector3              &origin  = {0, 0, 0});

  /** \brief Returns the snapshot data containing both mhd and raw files for given volume
   *
   *  \param[in] volume volume to create the snapshot from
   *  \param[in] path   storage relative path
   *  \param[in] id     storage base filename
   */
  template<typename T>
  Snapshot createSnapshot(typename T::Pointer   volume,
                          TemporalStorageSPtr   storage,
                          const QString        &path,
                          const QString        &id);

  template<typename T>
  typename T::Pointer readVolume(const QString &filename);

  /** \brief Returns the memory consumption in MB of a image given it's number of pixels.
   * \param[in] number_of_pixels.
   */
  template<typename T>
  double memory_size_in_MB(int number_of_pixels)
  {
    return number_of_pixels * sizeof(T) / 1024.0 / 1024.0;
  }

  /** \brief Helper method to return an itk image iterator for a given image and region.
   * \param[in] image itk image pointer.
   * \param[in] bounds region to iterate.
   *
   */
  template<typename T>
  itk::ImageRegionIterator<T> itkImageRegionIterator(typename T::Pointer image, const Bounds &bounds);

  /** \brief Helper method to return an itk image iterator for a given image and region.
   * \param[in] image itk image pointer.
   * \param[in] bounds region to iterate.
   *
   */
  template<typename T>
  itk::ImageRegionIteratorWithIndex<T> itkImageRegionIteratorWithIndex(typename T::Pointer image, const Bounds &bounds);

  /** \brief Changes the spacing of the block updating its origin position
   * \param[in] image index
   * \param[in] spacing to be changed
   *
   */
  template<typename T>
  void changeSpacing(typename T::Pointer image, typename T::SpacingType &spacing);

  /** \brief Changes the spacing of the image updating its origin position
   * \param[in] image index
   * \param[in] spacing to be changed
   * \param[in] ratio spacing conversion ration
   *
   */
  template<typename T>
  void changeSpacing(typename T::Pointer image, typename T::SpacingType &spacing, const NmVector3 &ratio);

  /** \brief Extracts a subimage from the source image.
   * \param[in] source source image.
   * \param[in] bounds bounds of the subimage to extract from source.
   */
  template<typename T>
  typename T::Pointer extract_image(typename T::Pointer const source, const VolumeBounds &bounds);

  /** \brief Copies a sub-image from the source image to the destination image.
   * \param[in] source source image.
   * \param[in] destination destination image.
   * \param[in] bounds bounds contained in both source and destination to copy.
   *
   */
  template<typename T>
  void copy_image(typename T::Pointer const source, typename T::Pointer destination, const Bounds &bounds);

  /** \brief Compares a region in two images and returns the ratio of identical voxels with some given value.
   * \param[in] image1 first image.
   * \param[in] image2 second image.
   * \param[in] bounds bounds contained in both images that define the block to compare.
   * \param[in] value scalar value to compare.
   *
   */
  template<typename T>
  unsigned long long compare_images(typename T::Pointer const image1, typename T::Pointer image2, const Bounds &bounds, typename T::ValueType value = SEG_VOXEL_VALUE);

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

  //-----------------------------------------------------------------------------
  template<typename T>
  void copy_image(typename T::Pointer const source, typename T::Pointer destination, const Bounds &bounds)
  {
    const auto sourcePointer = source->GetBufferPointer();
    const auto sourceLargest = source->GetLargestPossibleRegion();
    const auto sourceRegion  = equivalentRegion<T>(source, bounds);

    const auto componentsNum = source->GetNumberOfComponentsPerPixel();
    const auto copySize     = sourceRegion.GetSize(0)*componentsNum;

    const auto sourceZJump = sourceLargest.GetSize(0)*sourceLargest.GetSize(1)*componentsNum;
    const auto sourceYJump = sourceLargest.GetSize(0)*componentsNum;
    const auto sZStart     = sourceRegion.GetIndex(2)-sourceLargest.GetIndex(2);
    const auto sZEnd       = sZStart+sourceRegion.GetSize(2);
    const auto sYStart     = sourceRegion.GetIndex(1)-sourceLargest.GetIndex(1);
    const auto sYEnd       = sYStart+sourceRegion.GetSize(1);
    const auto sXStart     = (sourceRegion.GetIndex(0)-sourceLargest.GetIndex(0))*componentsNum;

    const auto destPointer   = destination->GetBufferPointer();
    const auto destLargest   = destination->GetLargestPossibleRegion();;
    const auto destRegion    = equivalentRegion<T>(destination, bounds);

    const auto destZJump = destLargest.GetSize(0)*destLargest.GetSize(1)*componentsNum;
    const auto destYJump = destLargest.GetSize(0)*componentsNum;
    const auto dZStart   = destRegion.GetIndex(2)-destLargest.GetIndex(2);
    const auto dYStart   = destRegion.GetIndex(1)-destLargest.GetIndex(1);
    const auto dXStart   = (destRegion.GetIndex(0)-destLargest.GetIndex(0))*componentsNum;

    for(int sZ = sZStart, dZ = dZStart; sZ < sZEnd; ++sZ, ++dZ)
    {
      for(int sY = sYStart, dY = dYStart; sY < sYEnd; ++sY, ++dY)
      {
        auto sPointer = sourcePointer + sZ*sourceZJump + sY*sourceYJump + sXStart;
        auto dPointer = destPointer + dZ*destZJump + dY*destYJump + dXStart;
        std::memcpy(dPointer, sPointer, copySize);
      }
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  unsigned long long compare_images(typename T::Pointer const image1, typename T::Pointer image2, const Bounds &bounds, typename T::ValueType value = SEG_VOXEL_VALUE)
  {
    if(!contains(equivalentBounds<T>(image1), bounds) || !contains(equivalentBounds<T>(image2),bounds))
    {
      qWarning() << "compare_images<T>() -> one or both images don't contain comparison bounds!";
      qWarning() << "image1" << equivalentBounds<T>(image1);
      qWarning() << "image2" << equivalentBounds<T>(image2);
      qWarning() << "common" << bounds;
      return 0;
    }

    auto iter1 = itkImageIterator<T>(image1, bounds);
    auto iter2 = itkImageIterator<T>(image2, bounds);

    unsigned long long identical = 0;

    while(!iter1.IsAtEnd())
    {
      if((iter1.Value() == value) && (iter2.Value() == value))
      {
        ++identical;
      }

      ++iter1;
      ++iter2;
    }

    return identical;
  }
}

#endif // ESPINA_VOLUMETRIC_DATA_UTILS_H
