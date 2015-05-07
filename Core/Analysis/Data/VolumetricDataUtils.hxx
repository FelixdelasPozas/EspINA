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

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

// ITK
#include <itkExtractImageFilter.h>
#include <itkImageToVTKImageFilter.h>

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
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds());

  /** \brief Draw @drawnVolume into @volume, resizing @volume bounds to fit @drawnVolume if necessary.
   * \param[in] volume VolumetricData pointer to expand and draw.
   * \param[in] drawnVolume itk image pointer to draw into @volume.
   * \param[in] bounds bounds object to add if necessary.
   *
   */
  template<typename T>
  void expandAndDraw(VolumetricData<T> *volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds());

  template<typename T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds());

  template<class T>
  void expandAndDraw(Output::WriteLockData<VolumetricData<T>> &volume, const BinaryMaskSPtr<unsigned char> &mask);

  /** \brief Resizes the image to the minimum bounds that can contain the volume.
   * \param[in] volume volume to transform.
   * \param[in] bgValue background value of the image.
   *
   *  The resultant image is always smaller or equal in size to the original one.
   */
  template<typename T>
  void fitToContents(VolumetricDataSPtr<T> volume, typename T::ValueType bgValue);


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
}

#include "Core/Analysis/Data/VolumetricDataUtils.cxx"

#endif // ESPINA_VOLUMETRIC_DATA_UTILS_H
