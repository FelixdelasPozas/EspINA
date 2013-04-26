/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef OUTPUTTYPE_H
#define OUTPUTTYPE_H

#include "Core/Model/Output.h"

#include <itkImageRegionIteratorWithIndex.h>

class vtkAlgorithmOutput;
class vtkDiscreteMarchingCubes;
class vtkImageConstantPad;
class vtkImplicitFunction;
class vtkPolyData;

namespace EspINA
{
  typedef itk::ImageRegionIteratorWithIndex<itkVolumeType>      itkVolumeIterator;
  typedef itk::ImageRegionConstIteratorWithIndex<itkVolumeType> itkVolumeConstIterator;

  typedef unsigned long long EspinaTimeStamp;

  class FilterOutput::OutputType
  : public QObject
  {
    static EspinaTimeStamp s_tick;
    Q_OBJECT
  public:
    virtual ~OutputType() {}

    virtual FilterOutput::OutputTypeName type() const = 0; 

    void setOutput(FilterOutput *output);

    virtual bool setInternalData(OutputTypeSPtr rhs) = 0;

    EspinaTimeStamp timeStamp()
    { return m_timeStamp; }

    virtual bool dumpSnapshot (const QString &prefix, Snapshot &snapshot) = 0;

    virtual bool fetchSnapshot(Filter *filter, const QString &prefix) = 0;

    /// Whether output has been manually edited
    virtual bool isEdited() const = 0;

    virtual bool isValid() const = 0;

    virtual FilterOutput::NamedRegionList editedRegions() const = 0;

    virtual void clearEditedRegions() = 0;

    virtual void dumpEditedRegions(const QString &prefix) const = 0;

    virtual void restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix) = 0;

  signals:
    void outputChanged();

  protected:
    explicit OutputType(FilterOutput *output)
    : m_output(output), m_timeStamp(s_tick++) {}

    void markAsModified() 
    {
      m_timeStamp = s_tick++;
    }

  protected:
    FilterOutput   *m_output;

  private:
    EspinaTimeStamp m_timeStamp;
  };

  class VolumeOutputType
  : public FilterOutput::OutputType
  {
  public:
    static const FilterOutput::OutputTypeName TYPE;

    typedef itkVolumeType::RegionType VolumeRegion;

  public:
    explicit VolumeOutputType(FilterOutput *output)
    : OutputType(output) {}

    virtual FilterOutput::OutputTypeName type() const
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

    virtual void addEditedRegion(const EspinaRegion &region) = 0;

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

    /// Equivalent to bounds method
    virtual EspinaRegion espinaRegion() const = 0;

    /// Largest possible region
    virtual VolumeRegion volumeRegion() const = 0;

    /// Volume's region equivalent to the normalized region
    virtual VolumeRegion volumeRegion(const EspinaRegion &region) const = 0;

    virtual itkVolumeIterator iterator() = 0;
    virtual itkVolumeIterator iterator(const EspinaRegion &region) = 0;

    virtual itkVolumeType::Pointer toITK() = 0;

    virtual const itkVolumeType::Pointer toITK() const = 0;

    virtual vtkAlgorithmOutput *toVTK() = 0;

    virtual const vtkAlgorithmOutput *toVTK() const = 0;

    virtual itkVolumeType::Pointer cloneVolume() const = 0;

    virtual itkVolumeType::Pointer cloneVolume(const EspinaRegion &region) const = 0;

    virtual itkVolumeType::Pointer cloneVolume(const VolumeRegion &region) const = 0;
  };

  typedef VolumeOutputType                  * VolumeOutputTypePtr;
  typedef boost::shared_ptr<VolumeOutputType> VolumeOutputTypeSPtr;

  VolumeOutputTypePtr  outputVolume(OutputPtr  output);
  VolumeOutputTypeSPtr outputVolume(OutputSPtr output);

  class MeshOutputType
  : public FilterOutput::OutputType
  {
  public:
    static const FilterOutput::OutputTypeName TYPE;

  public:
    explicit MeshOutputType(FilterOutput *output)
    : OutputType(output) {}

    virtual FilterOutput::OutputTypeName type() const
    { return TYPE; }

    virtual vtkAlgorithmOutput *mesh() const = 0;
  };

  typedef boost::shared_ptr<MeshOutputType> MeshOutputTypeSPtr;

  MeshOutputTypeSPtr meshOutput(OutputSPtr output);

} // namespace EspINA

#endif // OUTPUTTYPE_H
