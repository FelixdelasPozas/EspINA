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

#ifndef ESPINA_MESH_PROXY_H
#define ESPINA_MESH_PROXY_H

#include "Core/EspinaCore_Export.h"

// ESPAIN
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/DataProxy.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkImplicitFunction.h>

// C++
#include <memory>

namespace ESPINA
{
  class EspinaCore_EXPORT MeshProxy
  : public MeshData
  , public DataProxy
  {
  public:
    /** \brief MeshProxy class constructor.
     *
     */
    explicit MeshProxy()
    {}

    /** \brief MeshProxy class virtual destructor.
     *
     */
    virtual ~MeshProxy()
    {}

    virtual void set(DataSPtr data)
    {
      m_data = std::dynamic_pointer_cast<MeshData>(data);
      m_data->setOutput(this->m_output);
    }

    virtual Bounds bounds() const override
    { return m_data->bounds(); }

    virtual void setSpacing(const NmVector3& spacing)
    { m_data->setSpacing(spacing); }

    virtual NmVector3 spacing() const
    { return m_data->spacing(); }

    virtual TimeStamp lastModified() const override
    { return m_data->lastModified(); }

    virtual BoundsList editedRegions() const override
    { return m_data->editedRegions(); }

    virtual void setEditedRegions(const BoundsList& regions)
    { m_data->setEditedRegions(regions); }

    virtual void clearEditedRegions() override
    { m_data->clearEditedRegions(); }

    virtual bool isValid() const
    { return m_data->isValid(); }

    virtual bool isEmpty() const
    { return m_data->isEmpty(); }

    virtual Snapshot snapshot(TemporalStorageSPtr storage,
                              const QString      &path,
                              const QString      &id) const override
    { return m_data->snapshot(storage, path, id); }

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override
   { return m_data->editedRegionsSnapshot(storage, path, id); }

   virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)
   { return m_data->restoreEditedRegions(storage, path, id); }

    virtual size_t memoryUsage() const
    { return m_data->memoryUsage(); }

    virtual vtkSmartPointer<vtkPolyData> mesh() const       override
    { return m_data->mesh(); }

    virtual void setMesh(vtkSmartPointer<vtkPolyData> mesh) override
    { m_data->setMesh(mesh); }

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id) override
    { return m_data->fetchData(); }

  private:
    virtual QList<Data::Type> updateDependencies() const override
    { return m_data->dependencies(); }

  private:
    MeshDataSPtr m_data;

    friend class Output;
  };

  using MeshProxyPtr = MeshProxy *;
  using MeshProxySPtr = std::shared_ptr<MeshProxy>;

} // namespace ESPINA

#endif // ESPINA_MESH_PROXY_H
