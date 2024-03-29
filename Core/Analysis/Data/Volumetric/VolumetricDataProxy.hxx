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
  class VolumetricDataProxy
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

    virtual void set(DataSPtr data) override
    {
      if (m_data)
      {
        data->copyFetchContext(m_data);
      }
      m_data = std::dynamic_pointer_cast<VolumetricData<T>>(data);
    }

    virtual void setFetchContext(const TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
    { m_data->setFetchContext(storage, path, id, bounds); }

    virtual bool needFetch() const override
    { return m_data->needFetch(); }

    virtual size_t memoryUsage() const override
    {
      return m_data->memoryUsage();
    }

    virtual VolumeBounds bounds() const override
    {
      return m_data->bounds();
    }

    virtual void setOrigin(const NmVector3& origin) override
    {
      m_data->setOrigin(origin);
    }

    virtual void setSpacing(const NmVector3& spacing) override
    {
      m_data->setSpacing(spacing);
    }

    virtual const typename T::Pointer itkImage() const override
    {
      return m_data->itkImage();
    }

    virtual const typename T::Pointer itkImage(const Bounds& bounds) const override
    {
      return m_data->itkImage(bounds);
    }

    virtual void setBackgroundValue(const typename T::ValueType value) override
    {
      m_data->setBackgroundValue(value);
    }

    typename T::ValueType backgroundValue() const override
    {
      return m_data->backgroundValue();
    }

    virtual void draw(vtkImplicitFunction*        brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value)                   override
    {
      m_data->draw(brush, bounds, value);
    }

    virtual void draw(const typename T::Pointer volume)                    override
    {
      m_data->draw(volume);
    }

    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds)                    override
    {
      m_data->draw(volume, bounds);
    }

    virtual void draw(const typename T::IndexType &index,
                      const typename T::PixelType  value = SEG_VOXEL_VALUE) override
    {
      m_data->draw(index, value);
    }

    virtual void draw(const Bounds               &bounds,
                      const typename T::PixelType value = SEG_VOXEL_VALUE) override
    {
      m_data->draw(bounds, value);
    }

    virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) override
    {
     m_data->draw(mask, value);
    }

    virtual void resize(const Bounds &bounds) override
    {
      m_data->resize(bounds);
    }

    virtual const typename T::RegionType itkRegion() const override
    {
      return m_data->itkRegion();
    }

    virtual const typename T::SpacingType itkSpacing() const override
    {
      return m_data->itkSpacing();
    }

    virtual const typename T::PointType itkOriginalOrigin() const override
    {
      return m_data->itkOriginalOrigin();
    }

    virtual TimeStamp lastModified() const override
    {
      return m_data->lastModified();
    }

    virtual BoundsList editedRegions() const override
    {
      return m_data->editedRegions();
    }

    virtual void setEditedRegions(const BoundsList& regions) override
    {
      m_data->setEditedRegions(regions);
    }

    virtual void clearEditedRegions() override
    {
      m_data->clearEditedRegions();
    }

    virtual bool isValid() const override
    {
      return m_data->isValid();
    }

    virtual bool isEmpty() const override
    {
      return m_data->isEmpty();
    }

    virtual Snapshot snapshot(TemporalStorageSPtr storage,
                              const QString      &path,
                              const QString      &id) override
    {
      return m_data->snapshot(storage, path, id);
    }

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage,
                                           const QString      &path,
                                           const QString      &id) override
    {
      return m_data->editedRegionsSnapshot(storage, path, id);
    }

    virtual void restoreEditedRegions(TemporalStorageSPtr storage,
                                      const QString      &path,
                                      const QString      &id)            override
    {
      m_data->restoreEditedRegions(storage, path, id);
    }

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
    { return m_data->fetchData(); }

  private:
    virtual QList<Data::Type> updateDependencies() const override
    { return m_data->dependencies(); }


  private:
    std::shared_ptr<VolumetricData<T>> m_data;

    friend class Output;
  };
} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_DATA_PROXY_H
