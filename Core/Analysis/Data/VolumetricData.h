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

#ifndef ESPINA_VOLUMETRIC_DATA_H
#define ESPINA_VOLUMETRIC_DATA_H

#include "EspinaCore_Export.h"

#include "Core/Analysis/Data.h"
#include "Core/Analysis/DataProxy.h"

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include <memory>

class vtkImplicitFunction;
namespace EspINA
{
  template<typename T>
  class EspinaCore_EXPORT VolumetricData
  : public Data
  {
  public:
    static const Data::Type TYPE;

    using itkImageSPtr = typename T::Pointer;

  public:
    explicit VolumetricData() {}
    virtual ~VolumetricData() {}

    virtual Data::Type type() const
    { return TYPE; }

    virtual DataProxySPtr createProxy() const;

    /** \brief Return memory usage in MB
     *
     * Returns the amount of memory allocated to hold the volume representation
     */
    virtual double memoryUsage() const = 0;

    virtual void setOrigin(const typename T::PointType origin) = 0;

    virtual typename T::PointType origin() const = 0;

    virtual void setSpacing(const typename T::SpacingType spacing) = 0;

    virtual typename T::SpacingType spacing() const = 0;

    /** \brief Return a read only ItkImage equivalent to the whole volume representation.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const itkImageSPtr itkImage() const  = 0;

    /** \brief Return a read only ItkImage of volume representation contained in bounds.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const itkImageSPtr itkImage(const Bounds& bounds) const = 0;

    /** \brief Set volume background value
     *
     */
    virtual void setBackgroundValue(const typename T::ValueType value)
    {  m_bgValue = value; }

    /** \brief Return volume background value
     *
     */
    virtual typename T::ValueType backgroundValue() const
    {  return m_bgValue; }

    /** \brief Change every voxel value which satisfies the implicit function to the value given as parameter
     *
     *  If given bounds are not contained inside the volume bounds, the intersection will be applied
     */
    virtual void draw(const vtkImplicitFunction* brush,
                      const Bounds&      bounds,
                      const typename T::ValueType value) = 0;

    virtual void draw(const itkImageSPtr volume,
                      const Bounds&      bounds = Bounds()) = 0;

    /// Set voxels at index to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(itkVolumeType::IndexType index,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE) = 0;

    /** \brief Resize the volume to the minimum bounds containing all non background values
     *
     */
    virtual void fitToContent() = 0;

    /** \brief Resize the volume to the given bounds
     *
     *  New voxels will be set to background value
     */
    virtual void resize(const Bounds &bounds) = 0;

    /** \brief Undo last edition operation
     *
     */
    virtual void undo() = 0;

  private:
    typename T::ValueType m_bgValue;

    friend class Output;
  };

  template<typename T>
  const Data::Type VolumetricData<T>::TYPE = "VolumetricData";

  template< class T > using VolumetricDataPtr = VolumetricData<T> *;
  template< class T > using VolumetricDataSPtr = std::shared_ptr<VolumetricData<T>>;

  template< class T > VolumetricDataPtr<T> EspinaCore_EXPORT volumetricData(OutputPtr output); //NOTE: Use viewitem??
  template< class T > VolumetricDataSPtr<T> EspinaCore_EXPORT volumetricData(OutputSPtr output);

//   /// Get the vtk-equivalent extent defining the volume
//   void extent(int out[6]) const = 0;
// 
// 
//   /** \brief Volume's voxel's index at given spatial position
//    *
//    *  It doesn't check whether the index is valid or not
//    */
//   typename T::IndexType index(Nm x, Nm y, Nm z) = 0;
// 
//   /// Set voxels at coordinates (x,y,z) to value
//   ///NOTE: Current implementation will expand the image
//   ///      when drawing with value != 0
//   void draw(Nm x, Nm y, Nm z,
//             itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//             bool emitSignal = true) = 0;
// 
// 
//   /// Set voxels inside contour to value
//   ///NOTE: Current implementation will expand the image
//   ///      when drawing with value != 0
//   void draw(vtkPolyData *contour,
//             Nm slice,
//             PlaneType plane,
//             itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//             bool emitSignal = true) = 0;
// 
//   //NOTE: To Deprecate?
//   /// Fill output's volume with given value
//   void fill(itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//                     bool emitSignal = true) = 0;
// 
//   /// Fill output's volume's region with given value
//   void fill(const EspinaRegion &region,
//                     itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//                     bool emitSignal = true) = 0;
// 
//   using vtkImageSPtr = vtkSmartPointer<vtkImageData>;
// 
//   // NOTE: Needs to steal pointer from itkImage
//   const vtkImageSPtr vtkImage() const = 0;
//   const vtkImageSPtr vtkImage(const Bounds& bounds) const = 0;
//     virtual itkVolumeIterator iterator() = 0;
//     virtual itkVolumeIterator iterator(const EspinaRegion &region) = 0;
// 
//     virtual itkVolumeConstIterator constIterator() = 0;
//     virtual itkVolumeConstIterator constIterator(const EspinaRegion &region) = 0;

  //TODO: Templatizate
//   using VolumetricDataPtr  = VolumetricData*;
//   using VolumetricDataSPtr = std::shared_ptr<VolumetricData>;
// 
//   VolumetricDataPtr  EspinaCore_EXPORT volumetricData(OutputPtr  output); //NOTE: Use viewitem??
//   VolumetricDataSPtr EspinaCore_EXPORT volumetricData(OutputSPtr output);

    //bool collision(VolumetricDataSPtr volume) = 0;

} // namespace EspINA

#endif // ESPINA_VOLUMETRIC_DATA_H
