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


#ifndef ESPINAVOLUME_H
#define ESPINAVOLUME_H

#include "Core/EspinaTypes.h"
#include "Core/EspinaRegion.h"

#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageToVTKImageFilter.h>

#include <boost/shared_ptr.hpp>

class vtkAlgorithmOutput;

namespace EspINA
{
  typedef itk::ImageRegionIteratorWithIndex<itkVolumeType>      itkVolumeIterator;
  typedef itk::ImageRegionConstIteratorWithIndex<itkVolumeType> itkVolumeConstIterator;

  class EspinaVolume
  {
  public:
    typedef itkVolumeType::RegionType       VolumeRegion;
    typedef boost::shared_ptr<EspinaVolume> Pointer;

    explicit EspinaVolume(){}
    explicit EspinaVolume(itkVolumeType::Pointer volume);
    explicit EspinaVolume(const EspinaRegion &region, itkVolumeType::SpacingType spacing);
    virtual ~EspinaVolume(){}

    EspinaVolume operator=(itkVolumeType::Pointer volume);
    void setVolume(itkVolumeType::Pointer volume, bool disconnect=false);

    /// Volume's voxel's index at given spatial position
    /// It doesn't check whether the index is valid or not
    itkVolumeType::IndexType index(Nm x, Nm y, Nm z);

    /// Get the vtk-equivalent extent defining the volume
    void extent(int out[6]) const;
    /// Get the vtk-equivalent bounds defining the volume
    void bounds(double out[6]) const;
    ///
    void spacing(double out[3]) const;

    EspinaRegion espinaRegion();/// Equivale al bounds que hay arriba

    VolumeRegion volumeRegion();/// Largest possible region
    VolumeRegion volumeRegion(const EspinaRegion &region); /// La region del volumen que equivale a la region "normalizada"

    itkVolumeIterator iterator();
    itkVolumeIterator iterator(const EspinaRegion &region);

    itkVolumeConstIterator constIterator();
    itkVolumeConstIterator constIterator(const EspinaRegion &region);

    itkVolumeType::Pointer toITK();
    const itkVolumeType::Pointer toITK() const;

    vtkAlgorithmOutput *toVTK();
    const vtkAlgorithmOutput *toVTK() const;

    void update();

    /// Expands the volume to contain @region.
    void expandToFitRegion(EspinaRegion region);

  private:
    explicit EspinaVolume(const VolumeRegion &region, itkVolumeType::SpacingType spacing);

    VolumeRegion volumeRegion(EspinaRegion region, itkVolumeType::SpacingType spacing);

  protected:
    itkVolumeType::Pointer m_volume;

    // itk to vtk filter
    typedef itk::ImageToVTKImageFilter<itkVolumeType> itk2vtkFilterType;
    mutable itk2vtkFilterType::Pointer itk2vtk;
  };


  class ChannelVolume
  : public EspinaVolume
  {
  public:
    typedef boost::shared_ptr<ChannelVolume> Pointer;
  public:
    explicit ChannelVolume(itkVolumeType::Pointer volume);
    explicit ChannelVolume(const EspinaRegion& region, itkVolumeType::SpacingType spacing);
    virtual ~ChannelVolume(){}
  };

  /// Reduce volume dimensions to adjust it to the bounding box of the
  /// contained segmentation
  itkVolumeType::Pointer strechToFitContent(itkVolumeType::Pointer volume);

  class SegmentationVolume
  : public EspinaVolume
  {
  public:
    typedef boost::shared_ptr<SegmentationVolume> Pointer;

  public:
    explicit SegmentationVolume(itkVolumeType::Pointer volume);
    explicit SegmentationVolume(const EspinaRegion& region, itkVolumeType::SpacingType spacing);
    virtual ~SegmentationVolume(){}

    bool collision(SegmentationVolume v);

    /// Reduce volume dimensions to adjust it to the bounding box of the
    /// contained segmentation
    bool strechToFitContent();

  };

} // namespace EspINA

#endif // ESPINAVOLUME_H
