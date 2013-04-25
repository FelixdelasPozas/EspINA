/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef VOLUMEOUTPUTTYPE_H
#define VOLUMEOUTPUTTYPE_H

// EspINA
#include "Core/EspinaTypes.h"
#include "Core/EspinaRegion.h"
#include "Core/Model/OutputType.h"

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageToVTKImageFilter.h>

// VTK
#include <vtkSmartPointer.h>

// boost
#include <boost/shared_ptr.hpp>

class vtkAlgorithmOutput;
class vtkDiscreteMarchingCubes;
class vtkImageConstantPad;
class vtkImplicitFunction;
class vtkPolyData;

namespace EspINA
{
  typedef itk::ImageRegionIteratorWithIndex<itkVolumeType>      itkVolumeIterator;
  typedef itk::ImageRegionConstIteratorWithIndex<itkVolumeType> itkVolumeConstIterator;

  class VolumeOutputType
  : public FilterOutput::OutputType
  {
    Q_OBJECT
  public:
    static const FilterOutput::OutputTypeName TYPE;

    typedef itkVolumeType::RegionType           VolumeRegion;

    explicit VolumeOutputType(FilterOutput *output = NULL);
    explicit VolumeOutputType(itkVolumeType::Pointer volume,
                              FilterOutput *output = NULL);
    explicit VolumeOutputType(const EspinaRegion &region,
                              itkVolumeType::SpacingType spacing,
                              FilterOutput *output = NULL);
    virtual ~VolumeOutputType(){}

    virtual FilterOutput::OutputTypeName type() const
    { return TYPE; }

    virtual bool setInternalData(FilterOutput::OutputTypeSPtr rhs);

    /// Return the time stamp of the last output modification
    virtual long unsigned int timeStamp();

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
    void fill(const EspinaRegion &region,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot);
    virtual bool fetchSnapshot(const QString &prefix);

    virtual bool isEdited() const
    {return !m_editedRegions.isEmpty();}

    virtual bool isValid() const
    { return m_volume.IsNotNull(); }

    void addEditedRegion(const EspinaRegion &region);

    virtual FilterOutput::NamedRegionList editedRegions() const;

    virtual void clearEditedRegions();

    virtual void dumpEditedRegions(const QString &prefix) const;

    virtual void restoreEditedRegion(const EspinaRegion &region, const QString &prefix);

    //VolumeOutputType operator=(itkVolumeType::Pointer volume);
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

    virtual EspinaRegion espinaRegion() const;/// Equivalent to bounds method

    virtual VolumeRegion volumeRegion() const;/// Largest possible region
    virtual VolumeRegion volumeRegion(const EspinaRegion &region) const; /// Volume's region equivalent to the normalized region

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
    virtual void update();

    /// Expands the volume to contain @region.
    virtual void expandToFitRegion(EspinaRegion region);

  signals:
    void modified();

  private:
    explicit VolumeOutputType(const VolumeRegion &region,
                              itkVolumeType::SpacingType spacing,
                              FilterOutput *output);

    VolumeRegion volumeRegion(EspinaRegion region, itkVolumeType::SpacingType spacing) const;

  protected:
    mutable itkVolumeType::Pointer m_volume;
    QList<EspinaRegion>            m_editedRegions;

    // itk to vtk filter
    typedef itk::ImageToVTKImageFilter<itkVolumeType> itk2vtkFilterType;
    mutable itk2vtkFilterType::Pointer itk2vtk;

    mutable unsigned long int m_VTKGenerationTime;
    mutable unsigned long int m_ITKGenerationTime;
  };

  typedef boost::shared_ptr<VolumeOutputType> VolumeOutputTypeSPtr;

  VolumeOutputTypeSPtr volumeOutput(OutputSPtr output);

  class ChannelVolumeType
  : public VolumeOutputType
  {
  public:
    explicit ChannelVolumeType(itkVolumeType::Pointer volume,
                           FilterOutput *output = NULL);
    explicit ChannelVolumeType(const EspinaRegion& region,
                           itkVolumeType::SpacingType spacing,
                           FilterOutput *output = NULL);
    virtual ~ChannelVolumeType(){}
  };

  typedef boost::shared_ptr<ChannelVolumeType> ChannelVolumeTypeSPtr;

  ChannelVolumeTypeSPtr channelVolumeOutput(OutputSPtr output);

  class SegmentationVolumeType
  : public VolumeOutputType
  {
  public:
    explicit SegmentationVolumeType(FilterOutput *output = NULL);
    explicit SegmentationVolumeType(itkVolumeType::Pointer volume,
                                FilterOutput *output = NULL);
    explicit SegmentationVolumeType(const EspinaRegion& region,
                                itkVolumeType::SpacingType spacing,
                                FilterOutput *output = NULL);
    virtual ~SegmentationVolumeType(){}

    bool collision(SegmentationVolumeType v);

    /// Reduce volume dimensions to adjust it to the bounding box of the
    /// contained segmentation
    virtual bool fitToContent() throw(itk::ExceptionObject);

//     // get mesh representation of the volume
//     virtual vtkAlgorithmOutput *toMesh();
// 
//   private:
//     // vtkPolydata generation filter
//     vtkSmartPointer<vtkImageConstantPad>      m_padfilter;
//     vtkSmartPointer<vtkDiscreteMarchingCubes> m_march;
  };

  typedef boost::shared_ptr<SegmentationVolumeType> SegmentationVolumeTypeSPtr;
  SegmentationVolumeTypeSPtr segmentationVolumeOutput(OutputSPtr output);

} // namespace EspINA

#endif // VOLUMEOUTPUTTYPE_H
