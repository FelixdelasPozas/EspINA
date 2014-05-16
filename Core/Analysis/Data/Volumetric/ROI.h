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

      /* \brief ROI class constructor from a templated mask.
       *
       */
      ROI(const BinaryMaskSPtr<unsigned char> mask, unsigned char value);

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
      template<class T> void applyROI(VolumetricData<T> &volume, const typename T::ValueType outsideValue) const;

      /** \brief Implements SparseVolume::draw(brush, bounds, value)
       *
       */
      void draw(const vtkImplicitFunction*  brush,
                const Bounds&               bounds,
                const unsigned char         value = SEG_VOXEL_VALUE);

      /** \brief Implements SparseVolume::draw(mask, value)
       *
       */
      void draw(const BinaryMaskSPtr<unsigned char> mask,
                const unsigned char                 value = SEG_VOXEL_VALUE);

      /** \brief Implements SparseVolume::draw(volume)
       *
       */
      void draw(const typename itkVolumeType::Pointer volume);

      /** \brief Implements SparseVolume::draw(volume, bounds)
       *
       */
      void draw(const typename itkVolumeType::Pointer volume,
                const Bounds&                         bounds);

      /** \brief Implements SparseVolume::draw(index, value)
       *
       */
      void draw(const typename itkVolumeType::IndexType index,
                const typename itkVolumeType::PixelType value = SEG_VOXEL_VALUE);

    private:
      bool m_isRectangular;
  };

  using ROIPtr  = ROI *;
  using ROISPtr = std::shared_ptr<ROI>;

  //-----------------------------------------------------------------------------
  template<class T>
  inline void ROI::applyROI(VolumetricData<T>& volume, const typename T::ValueType outsideValue) const
  {
    if(!intersect(bounds(), volume.bounds()))
    {
      // erase the image
      BinaryMaskSPtr<typename T::ValueType> mask = BinaryMaskSPtr<typename T::ValueType>{ new BinaryMask<typename T::ValueType>{volume.bounds(), volume.spacing(), volume.origin()}};
      // TODO -> set fg value of mask to outsideValue;
      volume.draw(mask, outsideValue);

      return;
    }

    auto intersectionBounds = intersection(bounds(), volume.bounds());

    // extract the intersection region
    auto image = volume.itkImage(intersectionBounds);

    // erase the rest of the voxels
    auto mask = BinaryMaskSPtr<typename T::ValueType>{ new BinaryMask<typename T::ValueType>{volume.bounds(), volume.spacing(), volume.origin()}};
    volume.draw(mask, outsideValue);

    BinaryMask<unsigned char>::const_region_iterator crit(this, intersectionBounds);
    crit.goToBegin();

    if(spacing() == volume.spacing())
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
      // mask interapolation needed, more costly
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

} // namespace EspINA
#endif // ESPINA_ROI_H_

