/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include <Core/EspinaCore_Export.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>

namespace EspINA
{
  class EspinaCore_EXPORT ROI
  : public SparseVolume<itkVolumeType>
  {
    public:
      /* \brief ROI class constructor from a VolumeBounds.
       *
       */
      ROI(const Bounds &bounds, const NmVector3 &spacing, const NmVector3 &origin);

      /* \brief ROI class virtual destructor.
       *
       */
      virtual ~ROI();

      /* \brief Returns true if the ROI is a rectangular area.
       *
       */
      bool isRectangular() const;

      /* \brief Applies the ROI to the volume passed as argument.
       *
       */
      template<class T> void applyROI(VolumetricData<T> &volume, const typename T::ValueType value) const;

      /** \brief Implements SparseVolume::draw(brush, bounds, value)
       *
       */
      template<class T> void draw(const vtkImplicitFunction*  brush,
                                  const Bounds&               bounds,
                                  const typename T::ValueType value = SEG_VOXEL_VALUE);

      /** \brief Implements SparseVolume::draw(mask, value)
       *
       */
      template<class T> void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                                  const typename T::ValueType value = SEG_VOXEL_VALUE);

      /** \brief Implements SparseVolume::draw(volume)
       *
       */
      template<class T> void draw(const typename T::Pointer volume);

      /** \brief Implements SparseVolume::draw(volume, bounds)
       *
       */
      template<class T> void draw(const typename T::Pointer volume,
                                  const Bounds&             bounds);

      /** \brief Implements SparseVolume::draw(index, value)
       *
       */
      template<class T> void draw(const typename T::IndexType index,
                                  const typename T::PixelType value = SEG_VOXEL_VALUE);

    private:
      bool m_isRectangular;
  };

  using ROIPtr  = ROI *;
  using ROISPtr = std::shared_ptr<ROI>;

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::applyROI(VolumetricData<T>& volume, const typename T::ValueType value) const
  {
    if(!intersect(bounds(), volume.bounds()))
    {
      // erase the image
      auto mask = BinaryMaskSPtr<T>{ new BinaryMask<T>{volume.bounds(), volume.spacing(), volume.origin()}};
      volume.draw(mask, SEG_VOXEL_VALUE);

      return;
    }

    auto intersectionBounds = intersection(bounds(), volume.bounds());

    // extract the intersection region
    auto image = volume.itkImage(intersectionBounds);

    // erase the rest of the voxels
    auto mask = BinaryMaskSPtr<T>{ new BinaryMask<T>{volume.bounds(), volume.spacing(), volume.origin()}};
    volume.draw(mask, value);

    BinaryMask<unsigned char>::const_region_iterator crit(this, intersectionBounds);
    crit.goToBegin();

    if(spacing() == volume.spacing())
    {
      itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
      it.Begin();

      while(!crit.isAtEnd())
      {
        if(!crit.isSet())
          it.Set(value);

        ++crit;
        ++it;
      }
    }
    else
    {
      // mask interapolation needed, more costly
      while(!crit.isAtEnd())
      {
        if(!crit.isSet())
        {
          auto center = crit.getCenter();
          auto region = equivalentRegion<T>(image, Bounds{center});
          itk::ImageRegionIterator<T> it(image, region);
          it = it.Begin();
          while(it != it.End())
          {
            it.Set(value);
            ++it;
          }
        }

        ++crit;
      }
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::draw(const vtkImplicitFunction* brush, const Bounds& bounds, const typename T::ValueType value)
  {
    m_isRectangular = false;
    SparseVolume::draw(brush, bounds, value);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::draw(const BinaryMaskSPtr<typename T::ValueType> mask, const typename T::ValueType value)
  {
    m_isRectangular = false;
    SparseVolume::draw(mask, value);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::draw(const typename T::Pointer volume)
  {
    m_isRectangular = false;
    SparseVolume::draw(volume);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::draw(const typename T::Pointer volume, const Bounds& bounds)
  {
    m_isRectangular = false;
    SparseVolume::draw(volume, bounds);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::draw(const typename T::IndexType index, const typename T::PixelType value)
  {
    m_isRectangular = false;
    SparseVolume::draw(index, value);
  }

} // namespace EspINA
#endif // ESPINA_ROI_H_

