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

#ifndef ESPINA_VOLUMETRIC_DATA_PROXY_H
#define ESPINA_VOLUMETRIC_DATA_PROXY_H

#include <Core/Analysis/DataProxy.h>
#include "Core/Analysis/Data.h"

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class vtkImplicitFunction;
namespace EspINA
{
  template <class T> class VolumetricData; 

  template<typename T>
  class EspinaCore_EXPORT VolumetricDataProxy
  : public VolumetricData<T>
  , public DataProxy
  {

  public:
    using itkImageSPtr = typename T::Pointer;

  public:
    explicit VolumetricDataProxy() {}
    virtual ~VolumetricDataProxy() {}

    virtual void set(DataSPtr data)
    { 
      m_data = std::dynamic_pointer_cast<VolumetricData<T>>(data);
      m_data->setOutput(this->m_output);
    }

    virtual DataSPtr get() const
    { return m_data; }

    /** \brief Return memory usage in bytes
     *
     * Returns the amount of memory allocated to hold the volume representation
     */
    virtual size_t memoryUsage() const
    { return m_data->memoryUsage(); }

    virtual Bounds bounds() const
    { return m_data->bounds(); }

    virtual void setOrigin(const NmVector3& origin)
    { m_data->setOrigin(origin); }

    virtual NmVector3 origin() const
    { return m_data->origin(); }

    virtual void setSpacing(const NmVector3& spacing)
    { m_data->setSpacing(spacing); }

    virtual NmVector3 spacing() const
    { return m_data->spacing(); }

    /** \brief Return a read only ItkImage equivalent to the whole volume representation.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const itkImageSPtr itkImage() const
    { return m_data->itkImage(); }

    /** \brief Return a read only ItkImage of volume representation contained in bounds.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const itkImageSPtr itkImage(const Bounds& bounds) const
    { return m_data->itkImage(bounds); }

    /** \brief Set volume background value
     *
     */
    virtual void setBackgroundValue(const typename T::ValueType value)
    {  m_data->setBackgroundValue(value); }

    /** \brief Return volume background value
     *
     */
    typename T::ValueType backgroundValue() const
    {  return m_data->backgroundValue(); }

    /** \brief Change every voxel value which satisfies the implicit function to the value given as parameter
     *
     *  If given bounds are not contained inside the volume bounds, the intersection will be applied
     */
    virtual void draw(const vtkImplicitFunction* brush,
                      const Bounds&      bounds,
                      const typename T::ValueType value)
    { m_data->draw(brush, bounds, value); }

    virtual void draw(const itkImageSPtr volume,
                      const Bounds&      bounds = Bounds())
    { m_data->draw(volume, bounds); }

    /// Set voxels at index to value
    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0
    virtual void draw(itkVolumeType::IndexType index,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE)
    { m_data->draw(index, value); }

    /** \brief Resize the volume to the minimum bounds containing all non background values
     *
     */
    virtual void fitToContent()
    { m_data->fitToContent(); }

    /** \brief Resize the volume to the given bounds
     *
     *  New voxels will be set to background value
     */
    virtual void resize(const Bounds &bounds)
    { m_data->resize(bounds); }

    /** \brief Undo last edition operation
     *
     */
    virtual void undo()
    { m_data->undo(); }

    virtual TimeStamp lastModified()
    { return m_data->lastModified(); }

    virtual BoundsList editedRegions() const
    { return m_data->editedRegions(); }

    virtual void clearEditedRegions()
    { m_data->clearEditedRegions(); }

    virtual bool isValid() const
    { return m_data->isValid(); }

    virtual bool fetchData(TemporalStorageSPtr storage, const QString& prefix)
    { return m_data->fetchData(storage, prefix); }

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const
    { return m_data->snapshot(storage, prefix); }

    virtual Snapshot editedRegionsSnapshot() const
    { return m_data->editedRegionsSnapshot(); }

  private:
    std::shared_ptr<VolumetricData<T>> m_data;

    friend class Output;
  };
} // namespace EspINA

#endif // ESPINA_VOLUMETRIC_DATA_PROXY_H
