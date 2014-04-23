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
      /* \brief ROI class constructor from a sparse volume.
       *
       */
      ROI(SparseVolumeSPtr volume);

      /* \brief ROI class constructor from a VolumeBounds.
       *
       * This constructor allows having a VOI without an image associated to it
       * but the VOI will be rectangular. Any modification using the draw methods
       * will associate an image to the VOI.
       */
      ROI(VolumeBounds bounds, NmVector3 spacing, NmVector3 origin);

      /* \brief ROI class virtual destructor.
       *
       */
      virtual ~ROI();

      /** \brief Returns the memory used to store the image in bytes
       */
      virtual size_t memoryUsage() const;

      /** \brief Returns the bounds of the volume.
       */
      virtual Bounds bounds() const
      { return m_bounds.bounds(); }

      /** \brief Set volume origin.
       */
      virtual void setOrigin(const NmVector3& origin);

      /** \brief Returns volume origin.
       */
      virtual NmVector3 origin() const
      { return m_origin; }

      /** \brief Set volume spacing.
       */
      virtual void setSpacing(const NmVector3& spacing);

      /** \brief Returns volume spacing.
       */
      virtual NmVector3 spacing() const
      { return m_spacing; }

      /** \brief Returns the equivalent itk image of the volume.
       */
      virtual const typename itkVolumeType::Pointer itkImage() const;

      /** \brief Returns the equivalent itk image of a region of the volume.
       */
      virtual const typename itkVolumeType::Pointer itkImage(const Bounds& bounds) const;

      /** \brief Method to modify the volume using a implicit function.
       *
       *  Change every voxel value which satisfies the implicit function to
       *  the value given as parameter.
       *
       *  Draw methods are constrained to sparse volume bounds.
       */
      virtual void draw(const vtkImplicitFunction*  brush,
                        const Bounds&               bounds,
                        const typename itkVolumeType::ValueType value = SEG_VOXEL_VALUE);

      /** \brief Method to modify the volume using a mask and a value.
       *
       *  Draw methods are constrained to sparse volume bounds.
       */
      virtual void draw(const BinaryMaskSPtr<typename itkVolumeType::ValueType> mask,
                        const typename itkVolumeType::ValueType value = SEG_VOXEL_VALUE);

      /** \brief Method to modify the volume using an itk image.
       *
       *  Draw methods are constrained to sparse volume bounds.
       */
      virtual void draw(const typename itkVolumeType::Pointer volume);

      /** \brief Method to modify the volume using a region of an itk image.
       *
       *  Draw methods are constrained to sparse volume bounds.
       */
      virtual void draw(const typename itkVolumeType::Pointer volume,
                        const Bounds&             bounds);

      /** \brief Method to modify a voxel of the volume using an itk index.
       *
       *  Draw methods are constrained to volume bounds.
       */
      virtual void draw(const typename itkVolumeType::IndexType index,
                        const typename itkVolumeType::PixelType value = SEG_VOXEL_VALUE);

      /** \brief Resizes the image to the minimum bounds that can contain the volume.
       *
       *  The resultant image is always smaller of equal in size to the original one.
       */
      virtual void fitToContent(){}

      /** \brief Resize the volume bounds. The given bounds must containt the original.
       */
      virtual void resize(const Bounds &bounds);

      /** \brief Method to undo the last change made to the volume.
       */
      virtual void undo();

      /** \brief Returns if the volume has been correctly initialized.
       */
      virtual bool isValid() const;

      /** \brief Try to load data from storage
       *
       *  Return if any data was fetched
       */
      virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix);

      /** \brief Persistent Interface to save the mask state.
       */
      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const;

      virtual Snapshot editedRegionsSnapshot() const { return Snapshot(); }
    protected:
    private:
      bool hasVolume;
  };

  using ROIPtr  = ROI *;
  using ROISPtr = std::shared_ptr<ROI>;

} // namespace EspINA
#endif // ESPINA_ROI_H_
