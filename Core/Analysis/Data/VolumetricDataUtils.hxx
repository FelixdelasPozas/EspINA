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
#include <Core/Utils/Bounds.h>
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/VolumeBounds.h>
#include "VolumetricData.hxx"

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

// ITK
#include <itkExtractImageFilter.h>
#include <itkImageToVTKImageFilter.h>

namespace ESPINA
{
  /** \brief Return the image region equivalent to the bounds.
   * \param[in] image, itk image raw pointer.
   * \param[in] bounds, bounds to translate.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   * TODO: how to proceed when bounds doesn't intersect with largest possible image region
   *       - we coud either return an invalid region or throw an execption
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const T *image, const Bounds& bounds);

  /** \brief Return the image reagion equivalent to the bounds.
   * \param[in] origin, origin of the region.
   * \param[in] spacing, spacing of the region.
   * \param[in] bounds, bounds to translate.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds);

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] large, itk image smart pointer.
   * \param[in] region, region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] origin, origin of the region.
   * \param[in] spacing, spacing of the region.
   * \param[in] region, region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] origin, origin of the the region.
   * \param[in] spacing, spacing of the region.
   * \param[in] region, region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image, itk image smart pointer.
   * \param[in] bounds, bounds to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const Bounds& bounds);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image, itk image smart pointer.
   * \param[in] region, region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] origin, origin of the bounds.
   * \param[in] spacing, spacing of the region.
   * \param[in] bounds, bounds to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds);

  /** \brief Return the bounds of the left slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds leftSliceBounds(const  T &volume);

  /** \brief Return the bounds of the right slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds rightSliceBounds(const  T &volume);

  /** \brief Return the bounds of the top slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds topSliceBounds(const  T &volume);

  /** \brief Return the bounds of the bottom slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds bottomSliceBounds(const  T &volume);

  /** \brief Return the bounds of the front slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds frontSliceBounds(const  T &volume);

  /** \brief Return the bounds of the back slice of given @volume.
   * \param[in] volume, itk image reference.
   *
   */
  template<typename T>
  Bounds backSliceBounds(const  T &volume);

  /** \brief Return the number of voxels in image whose value is @value.
	 * \param[in] image, itk image smart pointer.
	 * \param[in] value, desired value to count.
   *
   */
  template<typename T>
  unsigned long voxelCount(const typename T::Pointer image, const typename T::ValueType value);

  /** \brief Return the minimal bounds of image which contains voxels with values different from @value.
   * \param[in] image, itk image smart pointer.
   * \param[in] value, value to take into consideration.
   *
   */
  template<typename T>
  Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType value);

  /** \brief Transform NmVector to ItkSpacing.
   * \param[in] spacing, NmVector3 to translate.
   *
   */
  template<typename T>
  typename T::SpacingType ItkSpacing(const NmVector3& spacing);

  /** \brief Transform from SpacingType to NmVector3.
   * \param[in] itkSpacing, itk SpacingType object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::SpacingType itkSpacing);

  /** \brief Transform from PointType to NmVector3.
   * \param[in] itkPoint, itk Point object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::PointType itkPoint);

  /** \brief Return the vtkImageData of specified bounds equivalent to the itkImage.
   * \param[in] volume, itk image smart pointer to transform.
   * \param[in] inputBounds, bounds of the image to transform.
   *
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds);

  /** \brief Return the vtkImageData of specified bounds equivalent to the volumetric data.
   * \param[in] volume, VolumetricData smart pointer to transform.
   * \param[in] bounds, bounds of the image to transform.
   *
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(VolumetricDataSPtr<T> volume, const Bounds &bounds);

  /** \brief Return the vtkImageData of specified bounds equivalent to the volumetric data of the specified output.
   * \param[in] output, output containing the volumetric data.
   * \param[in] bounds, bounds of the output to transform.
   *
   */
  template<class T>
  vtkSmartPointer<vtkImageData> vtkImage(OutputSPtr output, const Bounds &bounds);

  /** \brief Volume's voxel's index at given spatial position.
   * \param[in] x, x coordinate.
   * \param[in] y, y coordinate.
   * \param[in] z, z coordinate.
   *
   *  It doesn't check whether the index is valid or not
   */
  template<typename T>
  typename T::IndexType index(Nm x, Nm y, Nm z);

  /** \brief Returns whether or not the voxel at @point is not background.
   * \param[in] volume, VolumetricData smart pointer.
   * \param[in] point, point to check.
   *
   */
  template<typename T>
  bool isSegmentationVoxel(const VolumetricDataSPtr<T> volume, const NmVector3 &point);

  /** \brief Draw @drawnVolume into @volume, resizing @volume bounds to fit @drawnVolume if necessary.
   * \param[in] volume, VolumetricData smart pointer to expand and draw.
   * \param[in] drawn>Volume, itk image smart pointer to draw into @volume.
   * \param[in] bounds, bounds object to add if necessary.
   *
   */
  template<typename T>
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds());

  /** \brief Resizes the image to the minimum bounds that can contain the volume.
   * \param[in] volume, volume to transform.
   * \param[in] bgValue, background value of the image.
   *
   *  The resultant image is always smaller or equal in size to the original one.
   */
  template<typename T>
  void fitToContents(VolumetricDataSPtr<T> volume, typename T::ValueType bgValue);

  /** \brief Returns a new itk image smart pointer of the given spacing and origin.
   * \param[in] origin, origin of the resultant image.
   * \param[in] spacing, spacing of the resultant image.
   *
   */
  template<typename T>
  typename T::Pointer define_itkImage(const NmVector3              &origin  = {0, 0, 0},
                                      const NmVector3              &spacing = {1, 1, 1});

  /** \brief Returns a new itk image smart pointer of the given spacing, origin and bounds, filled with @value.
   * \param[in] bounds, bounds of the resultant image.
   * \param[in] value, value to fill the image.
   * \param[in] spacing, spacing of the resultant image.
   * \param[in] origin, origin of the resultant image.
   *
   */
  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                 bounds,
  																	  const typename T::ValueType   value   = 0,
                                      const NmVector3              &spacing = {1, 1, 1},
                                      const NmVector3              &origin  = {0, 0, 0});

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
