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

#ifndef VOLUMEREPRESENTATION_H
#define VOLUMEREPRESENTATION_H

#include <Core/Model/OutputRepresentation.h>

namespace EspINA
{
  class VolumeRepresentation
  {
  public:
    typedef itkVolumeType::RegionType VolumeRegion;

  public:
    virtual ~VolumeRepresentation() {}

    virtual void setVolume(itkVolumeType::Pointer volume, bool disconnect=false) = 0;

    /// Volume's voxel's index at given spatial position
    /// It doesn't check whether the index is valid or not
    virtual itkVolumeType::IndexType index(Nm x, Nm y, Nm z) = 0;

    /// Get the vtk-equivalent extent defining the volume
    virtual void extent(int out[6]) const = 0;

    /// Get the vtk-equivalent bounds defining the volume
    virtual void bounds(double out[6]) const = 0;
    ///
    virtual void spacing(double out[3]) const = 0;

    virtual itkVolumeType::SpacingType spacing() const = 0;

    /// Equivalent to bounds method
    virtual EspinaRegion espinaRegion() const = 0;

    /// Largest possible region
    virtual VolumeRegion volumeRegion() const = 0;

    /// Volume's region equivalent to the normalized region
    virtual VolumeRegion volumeRegion(const EspinaRegion &region) const = 0;

    virtual itkVolumeIterator iterator() = 0;
    virtual itkVolumeIterator iterator(const EspinaRegion &region) = 0;

    virtual itkVolumeConstIterator constIterator() = 0;
    virtual itkVolumeConstIterator constIterator(const EspinaRegion &region) = 0;


    virtual itkVolumeType::Pointer toITK() = 0;

    virtual const itkVolumeType::Pointer toITK() const = 0;

    virtual vtkAlgorithmOutput *toVTK() = 0;

    virtual const vtkAlgorithmOutput *toVTK() const = 0;

  protected:
  };

  class ChannelVolume
  : public ChannelRepresentation
  , public VolumeRepresentation
  {
  public:
    static const FilterOutput::OutputRepresentationName TYPE;

  public:
    virtual ~ChannelVolume(){}

    FilterOutput::OutputRepresentationName type() const
    { return TYPE; }

  protected:
    explicit ChannelVolume(FilterOutput *output)
    : ChannelRepresentation(output) {}
  };

  typedef ChannelVolume                  * ChannelVolumePtr;
  typedef boost::shared_ptr<ChannelVolume> ChannelVolumeSPtr;

  ChannelVolumePtr  channelVolume(OutputPtr  output);
  ChannelVolumeSPtr channelVolume(OutputSPtr output);



  class SegmentationVolume;
  typedef SegmentationVolume                  * SegmentationVolumePtr;
  typedef boost::shared_ptr<SegmentationVolume> SegmentationVolumeSPtr;

  class SegmentationVolume
  : public SegmentationRepresentation
  , public VolumeRepresentation
  {
  public:
    static const FilterOutput::OutputRepresentationName TYPE;

    class EditedVolumeRegion
    : public FilterOutput::EditedRegion
    {
    public:
      EditedVolumeRegion(int id, const EspinaRegion &region)
      : EditedRegion(id, SegmentationVolume::TYPE, region){}

      virtual bool dump(QDir           cacheDir,
                        const QString &regionName,
                        Snapshot      &snapshot) const;

      itkVolumeType::Pointer Volume;
    };

    typedef boost::shared_ptr<EditedVolumeRegion> EditedVolumeRegionSPtr;
    typedef QList<EditedVolumeRegionSPtr>         EditedVolumeRegionSList;

  protected:
    static QString cachePath(const QString &fileName)
    { return QString("%1/%2").arg(SegmentationVolume::TYPE).arg(fileName); }

  public:
    FilterOutput::OutputRepresentationName type() const
    { return TYPE; }

    /// Set voxels belonging to the implicit function defined by brush to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(vtkImplicitFunction *brush,
                      const Nm bounds[6],
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    /// Set voxels at index to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(itkVolumeType::IndexType index,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    /// Set voxels at coordinates (x,y,z) to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(Nm x, Nm y, Nm z,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    /// Set voxels inside contour to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(vtkPolyData *contour,
                      Nm slice,
                      PlaneType plane,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    /// Draw volume on top of output's voulume
    virtual void draw(itkVolumeType::Pointer volume,
                      bool emitSignal = true) = 0;

    /// Fill output's volume with given value
    virtual void fill(itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    /// Fill output's volume's region with given value
    virtual void fill(const EspinaRegion &region,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true) = 0;

    virtual EditedVolumeRegionSList editedRegions() const = 0;

    virtual void setEditedRegions(EditedVolumeRegionSList regions) = 0;

    virtual void setVolume(itkVolumeType::Pointer volume, bool disconnect=false) = 0;

    virtual itkVolumeType::Pointer cloneVolume() const = 0;

    virtual itkVolumeType::Pointer cloneVolume(const EspinaRegion &region) const = 0;

    virtual itkVolumeType::Pointer cloneVolume(const VolumeRegion &region) const = 0;

    virtual bool collision(SegmentationVolumeSPtr segmentation) = 0;

    virtual bool fitToContent() throw(itk::ExceptionObject) = 0;

    virtual void markAsModified(bool emitSignal = true) = 0;

  protected:
    explicit SegmentationVolume(FilterOutput *output)
    : SegmentationRepresentation(output) {}
  };

  SegmentationVolumePtr  segmentationVolume(OutputPtr              output);
  SegmentationVolumeSPtr segmentationVolume(OutputSPtr             output);
  SegmentationVolumeSPtr segmentationVolume(SegmentationOutputSPtr output);

} // namespace EspINA

#endif // VOLUMEREPRESENTATION_H
