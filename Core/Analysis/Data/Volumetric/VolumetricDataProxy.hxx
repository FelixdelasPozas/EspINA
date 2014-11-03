/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/Analysis/DataProxy.h>
#include <Core/Utils/BinaryMask.hxx>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class vtkImplicitFunction;
namespace ESPINA
{
  template <class T> class VolumetricData;

  template<typename T>
  class EspinaCore_EXPORT VolumetricDataProxy
  : public VolumetricData<T>
  , public DataProxy
  {
  public:
    /** \brief VolumetricDataProxy class constructor.
     *
     */
    explicit VolumetricDataProxy()
    {}

    /** \brief VolumetricDataProxy class virtual destructor.
     *
     */
    virtual ~VolumetricDataProxy()
    {}

    virtual void set(DataSPtr data)
    {
      m_data = std::dynamic_pointer_cast<VolumetricData<T>>(data);
      m_data->setOutput(this->m_output);
    }

    virtual void update() override
    { m_data->update();}

    virtual DataSPtr dynamicCast(DataProxySPtr proxy) const override
    { return std::dynamic_pointer_cast<VolumetricData<itkVolumeType>>(proxy); }

    virtual size_t memoryUsage() const
    { return m_data->memoryUsage(); }

    /** \brief Overrides VolumetricData<T>::bounds() const.
     *
     */
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

    virtual const typename T::Pointer itkImage() const
    { return m_data->itkImage(); }

    virtual const typename T::Pointer itkImage(const Bounds& bounds) const
    { return m_data->itkImage(bounds); }

    virtual void setBackgroundValue(const typename T::ValueType value)
    {  m_data->setBackgroundValue(value); }

    typename T::ValueType backgroundValue() const
    {  return m_data->backgroundValue(); }

    virtual void draw(const vtkImplicitFunction* brush,
                      const Bounds&      bounds,
                      const typename T::ValueType value)                   override
    { m_data->draw(brush, bounds, value); }

    virtual void draw(const typename T::Pointer volume)                    override
    { m_data->draw(volume); }

    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds)                    override
    { m_data->draw(volume, bounds); }

    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE) override
    { m_data->draw(index, value); }

    virtual void draw(const Bounds               &bounds,
                      const typename T::PixelType value = SEG_VOXEL_VALUE) override
    { m_data->draw(bounds, value); }


    virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) override
   { m_data->draw(mask, value); }

    virtual void resize(const Bounds &bounds)
    { m_data->resize(bounds); }

    virtual void undo()
    { m_data->undo(); }

    virtual TimeStamp lastModified()
    { return m_data->lastModified(); }

    virtual BoundsList editedRegions() const override
    { return m_data->editedRegions(); }

    virtual void setEditedRegions(const BoundsList& regions) override
    { m_data->setEditedRegions(regions); }

    virtual void clearEditedRegions() override
    { m_data->clearEditedRegions(); }

    virtual bool isValid() const
    { return m_data->isValid(); }

    virtual bool isEmpty() const
    { return m_data->isEmpty(); }

    virtual bool fetchData() override
    { return m_data->fetchData(); }

    virtual Snapshot snapshot(TemporalStorageSPtr storage,
                              const QString      &path,
                              const QString      &id) const              override
    { return m_data->snapshot(storage, path, id); }

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage,
                                           const QString      &path,
                                           const QString      &id) const override
    { return m_data->editedRegionsSnapshot(storage, path, id); }

    virtual void restoreEditedRegions(TemporalStorageSPtr storage,
                                      const QString      &path,
                                      const QString      &id)            override
    { return m_data->restoreEditedRegions(storage, path, id); }

  private:
    std::shared_ptr<VolumetricData<T>> m_data;

    friend class Output;
  };
} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_DATA_PROXY_H
