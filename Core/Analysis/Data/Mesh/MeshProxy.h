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

    /** \brief Implements DataProxy::set().
     *
     */
    virtual void set(DataSPtr data)
    {
      m_data = std::dynamic_pointer_cast<MeshData>(data);
      m_data->setOutput(this->m_output);
    }

    /** \brief Implements DataProxy::get().
     *
     */
    virtual DataSPtr get() const
    { return m_data; }

    /** \brief Overrides MeshData::bounds() const
     *
     */
    virtual Bounds bounds() const override
    { return m_data->bounds(); }

    /** \brief Implements Data::setSpacing().
     *
     */
    virtual void setSpacing(const NmVector3& spacing)
    { m_data->setSpacing(spacing); }

    /** \brief Implements Data::spacing().
     *
     */
    virtual NmVector3 spacing() const
    { return m_data->spacing(); }

    /** \brief Overrides Data::lastModified
     *
     */
    virtual TimeStamp lastModified() override
    { return m_data->lastModified(); }

    /** \brief Overrides Data::editedRegions().
     *
     */
    virtual BoundsList editedRegions() const override
    { return m_data->editedRegions(); }

    /** \brief Overrides Data::clearEditedRegions().
     *
     */
    virtual void clearEditedRegions() override
    { m_data->clearEditedRegions(); }

    /** \brief Implements Data::isValid().
     *
     */
    virtual bool isValid() const
    { return m_data->isValid(); }

    /** \brief Implements Data::isEmpty().
     *
     */
    virtual bool isEmpty() const
    { return m_data->isEmpty(); }

    /** \brief Implements Data::fetchData().
     *
     */
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString &path, const QString &id)
    { return m_data->fetchData(storage, path, id); }

    /** \brief Implements Data::snapshot().
     *
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
    { return m_data->snapshot(storage, path, id); }

    /** \brief Implements Data::editedRegionsSnapshot().
     *
     */
    virtual Snapshot editedRegionsSnapshot() const
    { return m_data->editedRegionsSnapshot(); }

    /** \brief Implements Data::undo().
     *
     */
    virtual void undo()
    { m_data->undo(); }

    /** \brief Implements Data::memoryUsage().
     *
     */
    virtual size_t memoryUsage() const
    { return m_data->memoryUsage(); }

    /** \brief MeshData::mesh().
     *
     */
    virtual vtkSmartPointer< vtkPolyData > mesh() const
    { return m_data->mesh(); }

  private:
    MeshDataSPtr m_data;

    friend class Output;
  };

  using MeshProxyPtr = MeshProxy *;
  using MeshProxySPtr = std::shared_ptr<MeshProxy>;

} // namespace ESPINA

#endif // ESPINA_MESH_PROXY_H
