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

  template< class T > VolumetricDataPtr<T>  EspinaCore_EXPORT volumetricData(OutputPtr output); //NOTE: Use viewitem??
  template< class T > VolumetricDataSPtr<T> EspinaCore_EXPORT volumetricData(OutputSPtr output);

  // TODO Delete later
  using DefaultVolumetricDataSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

} // namespace EspINA

#include "Core/Analysis/Data/VolumetricData.txx"

#endif // ESPINA_VOLUMETRIC_DATA_H
