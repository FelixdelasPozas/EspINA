/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef VOLUMEPROXY_H
#define VOLUMEPROXY_H

#include <Core/Outputs/VolumeRepresentation.h>
#include <Core/Outputs/VolumeRepresentation.h>

namespace EspINA
{
  class VolumeProxy 
  : public SegmentationVolume
  {
  public:
    explicit VolumeProxy(FilterOutput *output = NULL);

    virtual ~VolumeProxy(){}

    virtual bool setInternalData(SegmentationRepresentationSPtr rhs);

    /// Set voxels belonging to the implicit function defined by brush to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(vtkImplicitFunction *brush,
                      const Nm bounds[6],
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    /// Set voxels at index to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(itkVolumeType::IndexType index,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    /// Set voxels at coordinates (x,y,z) to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(Nm x, Nm y, Nm z,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    /// Set voxels inside contour to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(vtkPolyData *contour,
                      Nm slice,
                      PlaneType plane,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    /// Draw volume on top of output's voulume
    virtual void draw(itkVolumeType::Pointer volume,
                      bool emitSignal = true);

    /// Fill output's volume with given value
    virtual void fill(itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    /// Fill output's volume's region with given value
    virtual void fill(const EspinaRegion &region,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const;

    virtual bool isEdited() const;

    virtual bool isValid() const;

    virtual void addEditedRegion(const EspinaRegion &region);

    virtual void clearEditedRegions();

    virtual void commitEditedRegions(bool withData) const;

    virtual void restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix);

    virtual QList<EspinaRegion> editedRegions() const;

    virtual void setEditedRegions(QList<EspinaRegion> regions);

    virtual void setVolume(itkVolumeType::Pointer volume, bool disconnect=false);

    /// Volume's voxel's index at given spatial position
    /// It doesn't check whether the index is valid or not
    virtual itkVolumeType::IndexType index(Nm x, Nm y, Nm z);

    /// Get the vtk-equivalent extent defining the volume
    virtual void extent(int out[6]) const;

    /// Get the vtk-equivalent bounds defining the volume
    virtual void bounds(double out[6]) const;
    ///
    virtual void spacing(double out[3]) const;

    virtual itkVolumeType::SpacingType spacing() const;

    /// Equivalent to bounds method
    virtual EspinaRegion espinaRegion() const;

    /// Largest possible region
    virtual VolumeRegion volumeRegion() const;

    /// Volume's region equivalent to the normalized region
    virtual VolumeRegion volumeRegion(const EspinaRegion &region) const; 

    virtual itkVolumeIterator iterator();
    virtual itkVolumeIterator iterator(const EspinaRegion &region);

    virtual itkVolumeConstIterator constIterator();
    virtual itkVolumeConstIterator constIterator(const EspinaRegion &region);

    virtual itkVolumeType::Pointer toITK();
    virtual const itkVolumeType::Pointer toITK() const;

    virtual vtkAlgorithmOutput *toVTK();
    virtual const vtkAlgorithmOutput *toVTK() const;

    virtual itkVolumeType::Pointer cloneVolume() const;
    virtual itkVolumeType::Pointer cloneVolume(const EspinaRegion &region) const;
    virtual itkVolumeType::Pointer cloneVolume(const VolumeRegion &region) const;

    virtual void markAsModified(bool emitSignal = true);

    virtual bool fitToContent() throw(itk::ExceptionObject);

    virtual bool collision(SegmentationVolumeSPtr segmentation);
  private:
    SegmentationVolumeSPtr m_volumeRepresentation;
  };

  typedef VolumeProxy                  * VolumeProxyPtr;
  typedef boost::shared_ptr<VolumeProxy> VolumeProxySPtr;
}

#endif // VOLUMEPROXY_H
