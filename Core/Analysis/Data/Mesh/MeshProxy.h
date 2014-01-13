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

#ifndef ESPINA_MESH_PROXY_H
#define ESPINA_MESH_PROXY_H

#include "EspinaCore_Export.h"

#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/DataProxy.h>
#include <Core/Utils/Bounds.h>

#include <vtkImplicitFunction.h>

#include <memory>

namespace EspINA
{
  class EspinaCore_EXPORT MeshProxy
  : public MeshData
  , public DataProxy
  {
    public:
      explicit MeshProxy() {}
      virtual ~MeshProxy() {}

      virtual void set(DataSPtr data)
      {
        m_data = std::dynamic_pointer_cast<MeshData>(data);
        m_data->setOutput(this->m_output);
      }

      virtual DataSPtr get() const
      { return m_data; }


      virtual Bounds bounds() const
      { return m_data->bounds(); }

      virtual void setSpacing(const NmVector3& spacing)
      { m_data->setSpacing(spacing); }

      virtual NmVector3 spacing() const
      { return m_data->spacing(); }

      virtual TimeStamp lastModified()
      { return m_data->lastModified(); }

      virtual BoundsList editedRegions() const
      { return m_data->editedRegions(); }

      virtual void clearEditedRegions()
      { m_data->clearEditedRegions(); }

      virtual bool isValid() const
      { return m_data->isValid(); }

      virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix)
      { return m_data->fetchData(storage, prefix); }

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const
      { return m_data->snapshot(storage, prefix); }

      virtual Snapshot editedRegionsSnapshot() const
      { return m_data->editedRegionsSnapshot(); }

      virtual void undo()
      { m_data->undo(); }

      virtual size_t memoryUsage() const
      { return m_data->memoryUsage(); }

      virtual vtkSmartPointer< vtkPolyData > mesh() const
      { return m_data->mesh(); }
    private:
      MeshDataSPtr m_data;

      friend class Output;
    };

    using MeshProxyPtr = MeshProxy *;
    using MeshProxySPtr = std::shared_ptr<MeshProxy>;

  } // namespace EspINA

#endif // ESPINA_MESH_PROXY_H
