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

#ifndef ESPINA_SPATIAL_UTILS_H
#define ESPINA_SPATIAL_UTILS_H

// ESPINA
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Vector3.hxx>
#include <Core/Utils/VolumeBounds.h>

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
   * \param[in] image itk image raw pointer.
   * \param[in] bounds bounds to translate.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   * DESIGN: how to proceed when bounds doesn't intersect with largest possible image region
   *       - we coud either return an invalid region or throw an execption
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const T *image, const Bounds& bounds);

  /** \brief Return the image reagion equivalent to the bounds.
   * \param[in] origin origin of the region.
   * \param[in] spacing spacing of the region.
   * \param[in] bounds bounds to translate.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds);

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] large itk image smart pointer.
   * \param[in] region region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] origin origin of the region.
   * \param[in] spacing spacing of the region.
   * \param[in] region region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] origin origin of the the region.
   * \param[in] spacing spacing of the region.
   * \param[in] region region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image itk image smart pointer.
   * \param[in] bounds bounds to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const Bounds& bounds);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image itk image smart pointer.
   * \param[in] region region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the bounds of the left slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds leftSliceBounds(const  T &volume);

  /** \brief Return the bounds of the right slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds rightSliceBounds(const  T &volume);

  /** \brief Return the bounds of the top slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds topSliceBounds(const  T &volume);

  /** \brief Return the bounds of the bottom slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds bottomSliceBounds(const  T &volume);

  /** \brief Return the bounds of the front slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds frontSliceBounds(const  T &volume);

  /** \brief Return the bounds of the back slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds backSliceBounds(const  T &volume);

  /** \brief Return the number of voxels in image whose value is @value.
   * \param[in] image itk image smart pointer.
   * \param[in] value desired value to count.
   *
   */
  template<typename T>
  unsigned long voxelCount(const typename T::Pointer image, const typename T::ValueType value);

  /** \brief Return the minimal bounds of image which contains voxels with values different from @value.
   * \param[in] image itk image smart pointer.
   * \param[in] value value to take into consideration.
   *
   */
  template<typename T>
  Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType value);

  /** \brief Transform NmVector to ItkPointType
   * \param[in] point given as a NmVector3
   *
   */
  template<typename T>
  typename T::PointType ItkPoint(const NmVector3& point);

  /** \brief Transform NmVector to ItkSpacing.
   * \param[in] spacing NmVector3 to translate.
   *
   */
  template<typename T>
  typename T::SpacingType ItkSpacing(const NmVector3& spacing);

  /** \brief Transform from SpacingType to NmVector3.
   * \param[in] itkSpacing itk SpacingType object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::SpacingType itkSpacing);

  /** \brief Transform from PointType to NmVector3.
   * \param[in] itkPoint itk Point object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::PointType itkPoint);


  /** \brief Volume's voxel's index at given spatial position.
   * \param[in] x x coordinate.
   * \param[in] y y coordinate.
   * \param[in] z z coordinate.
   *
   *  It doesn't check whether the index is valid or not
   */
  template<typename T>
  typename T::IndexType index(Nm x, Nm y, Nm z);

  /** \brief Returns a new itk image smart pointer of the given spacing and origin.
   * \param[in] origin origin of the resultant image.
   * \param[in] spacing spacing of the resultant image.
   *
   */
  template<typename T>
  typename T::Pointer define_itkImage(const NmVector3              &origin  = {0, 0, 0},
                                      const NmVector3              &spacing = {1, 1, 1});

#include "Core/Utils/SpatialUtils.cxx"
}

#endif // ESPINA_SPATIAL_UTILS_H
