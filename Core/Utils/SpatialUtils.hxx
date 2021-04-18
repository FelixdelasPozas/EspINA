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
#include <Core/Utils/EspinaException.h>

// VTK
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

// ITK
#include <itkExtractImageFilter.h>
#include <itkImageToVTKImageFilter.h>

namespace ESPINA
{
  /** \brief Returns a new itk image smart pointer of the given spacing and origin.
   * \param[in] origin origin of the resultant image.
   * \param[in] spacing spacing of the resultant image.
   *
   */
  template<typename T>
  typename T::Pointer define_itkImage(const NmVector3 &origin,
                                      const NmVector3 &spacing)
  {
    typename T::PointType   itkOrigin;
    typename T::SpacingType itkSpacing;

    for(int i = 0; i < 3; ++i)
    {
      itkOrigin[i]  = origin[i];
      itkSpacing[i] = spacing[i];
    }

    typename T::Pointer image = T::New();
    // Origin and spacing must be set before calling equivalentRegion on image
    image->SetOrigin(itkOrigin);
    image->SetSpacing(itkSpacing);

    return image;
  }

  /** \brief Return the image region equivalent to the bounds.
   * \param[in] image itk image raw pointer.
   * \param[in] bounds bounds to translate.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   * DESIGN: how to proceed when bounds doesn't intersect with largest possible image region,
   *         we could either return an invalid region or throw an exception
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const T* image, const Bounds& bounds)
  {
    typename T::SpacingType s = image->GetSpacing();
    typename T::PointType   o = image->GetOrigin();

    NmVector3 hSpacing{s[0]/2.0, s[1]/2.0, s[2]/2.0};

    typename T::PointType p0, p1;
    for (int i = 0; i < 3; ++i)
    {
      p0[i] = bounds[2 * i]     + hSpacing[i] - o[i];
      p1[i] = bounds[2 * i + 1] - hSpacing[i] - o[i];

      if(!areEqual(std::remainder(p0[i],s[i]), 0, s[i]))
      {
        p0[i] -= hSpacing[i];
      }

      if(!areEqual(std::remainder(p1[i],s[i]), 0, s[i]))
      {
        p1[i] += hSpacing[i];
      }

      p0[i] += o[i];
      p1[i] += o[i];

      if(bounds[2*i] == bounds[2*i+1])
      {
        p1[i] = p0[i];
      }

      if(p0[i] > p1[i])
      {
        std::swap(p0[i], p1[i]);
      }
    }

    typename T::IndexType i0, i1;
    image->TransformPhysicalPointToIndex(p0, i0);
    image->TransformPhysicalPointToIndex(p1, i1);

    typename T::RegionType region;
    region.SetIndex(i0);
    region.SetUpperIndex(i1);

    for (auto i: {0,1,2})
    {
      if (bounds[2*i] == bounds[2*i+1])
      {
        region.SetSize(i,1);
      }
    }

    return region;
  }

  /** \brief Return the image region equivalent to the bounds.
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
  typename T::RegionType equivalentRegion(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    return equivalentRegion<T>(image, bounds);
  }

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] image to get bounds for
   * \param[in] region region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region)
  {
    Bounds bounds;

    typename T::PointType p0, p1;

    image->TransformIndexToPhysicalPoint(region.GetIndex(), p0);
    image->TransformIndexToPhysicalPoint(region.GetUpperIndex(), p1);

    typename T::SpacingType s = image->GetSpacing();

    for (int i = 0; i < 3; ++i)
    {
      bounds[2*i]   = p0[i] - s[i]/2;
      bounds[2*i+1] = p1[i] + s[i]/2;
    }

    return bounds;
  }

  /** \brief Return the bounds for the largest region of an image.
   * \param[in] image to get bounds for
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image)
  {
    return equivalentBounds<T>(image, image->GetLargestPossibleRegion());
  }

  /** \brief Return the bounds for a given image region and an image.
   * \param[in] origin origin of the region.
   * \param[in] spacing spacing of the region.
   * \param[in] region region to translate.
   *
   * Bounds are given in nm using (0,0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    return equivalentBounds<T>(image, region);
  }

  /** \brief Return the image region equivalent to the bounds.
   * \param[in] bounds VolumeBounds object.
   *
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const VolumeBounds& bounds)
  {
    typename T::Pointer image = define_itkImage<T>(bounds.origin(), bounds.spacing());

    return equivalentRegion<T>(image, bounds);
  }


  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image itk image smart pointer.
   * \param[in] bounds bounds to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const Bounds& bounds)
  {
    NmVector3 origin;
    for (int i = 0; i < 3; ++i) origin[i] = image->GetOrigin()[i];

    NmVector3 spacing;
    for (int i = 0; i < 3; ++i) spacing[i] = image->GetSpacing()[i];

    return VolumeBounds(bounds, spacing, origin);
  }

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image itk image smart pointer.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image)
  {
    return volumeBounds<T>(image, equivalentBounds<T>(image, image->GetLargestPossibleRegion()));
  }

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] image itk image smart pointer.
   * \param[in] region region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region)
  {
    return volumeBounds<T>(image, equivalentBounds<T>(image, region));
  }

  /** \brief Return the minimum complete bounds for any image of given origin and spacing.
   * \param[in] origin origin of the the region.
   * \param[in] spacing spacing of the region.
   * \param[in] region region to translate.
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region)
  {
    return volumeBounds<T>(define_itkImage<T>(origin, spacing), region);
  }

  /** \brief Return the bounds of the left slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds leftSliceBounds(const T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[1] = slice[0] + spacing[0]/2.0;

    return slice;
  }

  /** \brief Return the bounds of the right slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds rightSliceBounds(const  T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[0] = slice[1] - spacing[0]/2.0;

    return slice;
  }

  /** \brief Return the bounds of the top slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds topSliceBounds(const  T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[3] = slice[2] + spacing[1]/2.0;

    return slice;
  }

  /** \brief Return the bounds of the bottom slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds bottomSliceBounds(const  T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[2] = slice[3] - spacing[1]/2.0;

    return slice;
  }

  /** \brief Return the bounds of the front slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds frontSliceBounds(const  T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[5] = slice[4] + spacing[2]/2.0;

    return slice;
  }

  /** \brief Return the bounds of the back slice of given @volume.
   * \param[in] volume itk image reference.
   *
   */
  template<typename T>
  Bounds backSliceBounds(const  T &volume)
  {
    Bounds slice = volume->bounds();
    auto spacing = volume->bounds().spacing();

    slice[4] = slice[5] - spacing[2]/2.0;

    return slice;
  }

  /** \brief Return the number of voxels in image whose value is @value.
   * \param[in] image itk image smart pointer.
   * \param[in] value desired value to count.
   *
   */
  template<typename T>
  unsigned long voxelCount(const typename T::Pointer image, const typename T::ValueType value)
  {
    unsigned long count = 0;

    itk::ImageRegionConstIterator<T> it(image, image->GetLargestPossibleRegion());

    it.GoToBegin();
    while (!it.IsAtEnd())
    {
      if (it.Get()) ++count;
      ++it;
    }

    return count;
  }

  /** \brief Transform from SpacingType to NmVector3.
   * \param[in] itkSpacing itk SpacingType object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::SpacingType itkSpacing)
  {
    NmVector3 vector;

    for(int i = 0; i < 3; ++i)
    {
      vector[i] = itkSpacing[i];
    }

    return vector;
  }

  /** \brief Return the minimal bounds of image which contains voxels with values different from @value.
   * \param[in] image itk image smart pointer.
   * \param[in] value value to take into consideration.
   *
   */
  template<typename T>
  Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType bgValue)
  {
    itk::ImageRegionConstIterator<T> it(image, image->GetLargestPossibleRegion());
    int minIndex[3];
    int maxIndex[3];
    bool empty = true;

    it.GoToBegin();
    while (!it.IsAtEnd())
    {
      if (it.Get() != bgValue)
      {
        auto index   = it.GetIndex();
        if(empty)
        {
          minIndex[0] = index[0];
          minIndex[1] = index[1];
          minIndex[2] = index[2];
          maxIndex[0] = index[0];
          maxIndex[1] = index[1];
          maxIndex[2] = index[2];
          empty = false;
        }
        else
        {
          if(minIndex[0] > index[0]) minIndex[0] = index[0];
          if(minIndex[1] > index[1]) minIndex[1] = index[1];
          if(minIndex[2] > index[2]) minIndex[2] = index[2];
          if(maxIndex[0] < index[0]) maxIndex[0] = index[0];
          if(maxIndex[1] < index[1]) maxIndex[1] = index[1];
          if(maxIndex[2] < index[2]) maxIndex[2] = index[2];
        }
      }
      ++it;
    }

    // empty volume
    if(empty) return Bounds();

    auto origin   = image->GetOrigin();
    auto spacing  = image->GetSpacing();
    auto vSpacing = ToNmVector3<T>(spacing);

    Bounds minBounds, maxBounds;
    for (int i = 0; i < 3; ++i)
    {
      minBounds[2*i]   = ( minIndex[i]    * spacing[i]) - origin[i] - spacing[i]/2;
      minBounds[2*i+1] = ((minIndex[i]+1) * spacing[i]) - origin[i] - spacing[i]/2;
      maxBounds[2*i]   = ( maxIndex[i]    * spacing[i]) - origin[i] - spacing[i]/2;
      maxBounds[2*i+1] = ((maxIndex[i]+1) * spacing[i]) - origin[i] - spacing[i]/2;
    }

    return boundingBox(minBounds, maxBounds, vSpacing);
  }

  /** \brief Transform NmVector to ItkPointType
   * \param[in] point given as a NmVector3
   *
   */
  template<typename T>
  typename T::PointType ItkPoint(const NmVector3& point)
  {
    typename T::PointType itkPoint;

    for(int i = 0; i < 3; ++i)
    {
      itkPoint[i] = point[i];
    }

    return itkPoint;
  }

  /** \brief Transform NmVector to ItkSpacing.
   * \param[in] spacing NmVector3 to translate.
   *
   */
  template<typename T>
  typename T::SpacingType ItkSpacing(const NmVector3& spacing)
  {
    typename T::SpacingType itkSpacing;

    for(int i = 0; i < 3; ++i)
    {
      itkSpacing[i] = spacing[i];
    }

    return itkSpacing;
  }

  /** \brief Transform from PointType to NmVector3.
   * \param[in] itkPoint itk Point object to translate.
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::PointType itkPoint)
  {
    NmVector3 vector;

    for(int i = 0; i < 3; ++i)
    {
      vector[i] = itkPoint[i];
    }

    return vector;
  }

}

#endif // ESPINA_SPATIAL_UTILS_H
