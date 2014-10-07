/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ROI_H_
#define ESPINA_ROI_H_

// ESPINA
#include <Core/EspinaCore_Export.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>

// ITK
#include <itkImageRegionExclusionIteratorWithIndex.h>

namespace ESPINA
{
  class ROI;

  using ROIPtr  = ROI *;
  using ROISPtr = std::shared_ptr<ROI>;

  class EspinaCore_EXPORT ROI
  : public SparseVolume<itkVolumeType>
  {
    public:
      /** \brief ROI class constructor.
       * \param[in] bounds, initial bounds of the volume.
       * \param[in] spacing, spacing of the volume.
       * \param[in] origin, origin of the volume.
       *
       */
      ROI(const Bounds &bounds, const NmVector3 &spacing, const NmVector3 &origin);

      /** \brief ROI class constructor.
       * \param[in] mask, mask used as a volume.
       *
       */
      ROI(const BinaryMaskSPtr<unsigned char> mask);

      /** \brief ROI class virtual destructor.
       *
       */
      virtual ~ROI();

      /** \brief Returns true if the ROI is a rectangular area.
       *
       */
      bool isRectangular() const;

      /** \brief Returns a new ROI object that is a copy of this one.
       *
       */
      ROISPtr clone() const;

      /** \brief Applies the ROI to the volume passed as argument.
       * \param[in] volume, volumetricData to apply the ROI.
       * \param[in] outsideValue, value to be considered outside the ROI when applying it.
       *
       */
      template<class T> void applyROI(VolumetricDataSPtr<T> volume, const typename T::ValueType outsideValue) const;

      /** \brief Applies the ROI to the volume passed as argument.
       * \param[in] volume, itk volume smart pointer to apply the ROI.
       * \param[in] outsideValue, value to be considered outside the ROI when applying it.
       *
       */
      template<class T> void applyROI(typename T::Pointer volume, const typename T::ValueType outsideValue) const;

      /** \brief Overrides SparseVolume<T>::draw(vtkImplicitFunction, Bounds, T::valueType)
       *
       */
      void draw(const vtkImplicitFunction*     brush,
                const Bounds&                  bounds,
                const itkVolumeType::ValueType value = SEG_VOXEL_VALUE) override;

      /** \brief Overrides SparseVolume<T>::draw(BinaryMaskSPtr, T:ValueType)
       *
       */
      void draw(const BinaryMaskSPtr<unsigned char> mask,
                const itkVolumeType::ValueType      value = SEG_VOXEL_VALUE) override;

      /** \brief Overrides SparseVolume::draw(T::Pointer)
       *
       */
      void draw(const typename itkVolumeType::Pointer volume) override;

      /** \brief Overrides SparseVolume::draw(T::Pointer, bounds)
       *
       */
      void draw(const typename itkVolumeType::Pointer volume,
                const Bounds&                         bounds) override;

      /** \brief Overrides SparseVolume::draw(T::IndexType, T::ValueType)
       *
       */
      void draw(const typename itkVolumeType::IndexType index,
                const typename itkVolumeType::ValueType value = SEG_VOXEL_VALUE) override;

    private:
      bool m_isRectangular;
  };

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::applyROI(VolumetricDataSPtr<T> volume, const typename T::ValueType outsideValue) const
  {
    if(!intersect(bounds(), volume->bounds()))
    {
      // erase the image
      BinaryMaskSPtr<typename T::ValueType> mask = BinaryMaskSPtr<typename T::ValueType>{ new BinaryMask<typename T::ValueType>{volume->bounds(), volume->spacing(), volume->origin()}};
      mask->setForegroundValue(outsideValue);
      volume->draw(mask, outsideValue);

      return;
    }

    auto intersectionBounds = intersection(bounds(), volume->bounds());

    // extract the intersection region
    auto image = volume->itkImage(intersectionBounds);

    // erase the rest of the voxels
    auto mask = BinaryMaskSPtr<typename T::ValueType>{ new BinaryMask<typename T::ValueType>{volume->bounds(), volume->spacing(), volume->origin()}};
    volume->draw(mask, outsideValue);

    BinaryMask<unsigned char>::const_region_iterator crit(this, intersectionBounds);
    crit.goToBegin();

    if(spacing() == volume->spacing())
    {
      itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
      it.Begin();

      while(!crit.isAtEnd())
      {
        if(!crit.isSet())
          it.Set(outsideValue);

        ++crit;
        ++it;
      }
    }
    else
    {
      // mask interpolation needed, more costly
      auto spacing = spacing();

      while(!crit.isAtEnd())
      {
        if(!crit.isSet())
        {
          auto center = crit.getCenter();

          auto voxelBounds = Bounds{center[0]-spacing[0]/2, center[0]+spacing[0]/2,
                                    center[1]-spacing[1]/2, center[1]+spacing[1]/2,
                                    center[2]-spacing[2]/2, center[2]+spacing[2]/2};

          auto region = equivalentRegion<T>(image, voxelBounds);
          itk::ImageRegionIterator<T> it(image, region);
          it = it.Begin();
          while(it != it.End())
          {
            it.Set(outsideValue);
            ++it;
          }
        }

        ++crit;
      }
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::applyROI(typename T::Pointer volume, const typename T::ValueType outsideValue) const
  {
    auto imageBounds = equivalentBounds<T>(volume, volume->GetLargestPossibleRegion());
    auto intersectionBounds = intersection(imageBounds, bounds());

    if (!intersectionBounds.areValid())
    {
      // erase the image, the roi is outside the image
      itk::ImageRegionIterator<T> it(volume, volume->GetLargestPossibleRegion());
      it.GoToBegin();
      while (!it.IsAtEnd())
      {
        it.Set(outsideValue);
        ++it;
      }
      return;
    }

    // clear outside region
    if(imageBounds != bounds())
    {
      auto region = equivalentRegion<T>(volume, intersectionBounds);
      itk::ImageRegionExclusionIteratorWithIndex<T> it(volume, volume->GetLargestPossibleRegion());
      it.SetExclusionRegion(region);
      it.GoToBegin();
      while (!it.IsAtEnd())
      {
        it.Set(outsideValue);
        ++it;
      }
    }

    // if it's rectangular we're done now.
    if(!this->isRectangular())
    {
      auto image = this->itkImage(intersectionBounds);
      itk::ImageRegionIterator<itkVolumeType> it(image, image->GetLargestPossibleRegion());
      it.GoToBegin();

      auto volumeSpacing = volume->GetSpacing();
      auto roiSpacing = image->GetSpacing();

      if(roiSpacing == volumeSpacing)
      {
        auto region = equivalentRegion<T>(volume, intersectionBounds);
        itk::ImageRegionIterator<T> rit(volume, region);
        rit.GoToBegin();

        while(!it.IsAtEnd())
        {
          if(it.Value() != SEG_VOXEL_VALUE)
            rit.Set(outsideValue);

          ++it;
          ++rit;
        }
      }
      else
      {
        while(!it.IsAtEnd())
        {
          if(it.Value() != SEG_VOXEL_VALUE)
          {
            auto index = it.GetIndex();
            auto origin = image->GetOrigin();
            NmVector3 point{ (index[0]+origin[0]) * roiSpacing[0], (index[1]*origin[1]) * roiSpacing[1], (index[2]*origin[2]) * roiSpacing[2]};
            VolumeBounds vBounds{Bounds(point), this->spacing(), this->origin()};
            auto region = equivalentRegion<T>(volume, vBounds.bounds());
            itk::ImageRegionIterator<T> rit(volume, region);
            rit.GoToBegin();

            while(!rit.IsAtEnd())
            {
              rit.Set(outsideValue);
              ++rit;
            }
          }

          ++it;
        }
      }
    }
  }


  /** \brief Returns whether or not point is inside the roi
   *
   *  \param[in] roi region of interest
   *  \param[in] point point to be checked for inclussion inside the roi
   *  \param[in] spacing to determine whether two distances belong to the same voxel
   */
  bool contains(ROISPtr roi, NmVector3 point, NmVector3 spacing = NmVector3{1, 1, 1});
} // namespace ESPINA
#endif // ESPINA_ROI_H_

