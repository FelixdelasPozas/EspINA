/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SKELETON_PROXY_H_
#define ESPINA_SKELETON_PROXY_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/DataProxy.h>

// VTK
#include <vtkSmartPointer.h>

// C++
#include <memory>

class vtkPolyData;

namespace ESPINA
{
  class EspinaCore_EXPORT SkeletonProxy
  : public SkeletonData
  , public DataProxy
  {
    public:
      /** \brief SkeletonProxy class constructor.
       *
       */
      explicit SkeletonProxy()
      {}

      /** \brief SkeletonProxy class virtual destructor.
       *
       */
      virtual ~SkeletonProxy()
      {}

      virtual void set(DataSPtr data) override
      {
        if (m_data)
        {
          data->copyFetchContext(m_data);
        }
        m_data = std::dynamic_pointer_cast<SkeletonData>(data);
      }

      virtual void setFetchContext(const TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
      { return m_data->setFetchContext(storage, path, id, bounds); }

      virtual bool needFetch() const override
      { return m_data->needFetch(); }

      virtual VolumeBounds bounds() const override
      { return m_data->bounds(); }

      virtual void setSpacing(const NmVector3& spacing) override
      { m_data->setSpacing(spacing); }

      virtual TimeStamp lastModified() const override
      { return m_data->lastModified(); }

      virtual BoundsList editedRegions() const override
      { return m_data->editedRegions(); }

      virtual void clearEditedRegions() override
      { m_data->clearEditedRegions(); }

      virtual bool isValid() const override
      { return m_data->isValid(); }

      virtual bool isEmpty() const override
      { return m_data->isEmpty(); }

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override
      { return m_data->snapshot(storage, path, id); }

      virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override
      { return m_data->editedRegionsSnapshot(storage, path, id); }

      virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString &path, const QString &id) override
      { return m_data->restoreEditedRegions(storage, path, id); }

      virtual size_t memoryUsage() const override
      { return m_data->memoryUsage(); }

      virtual vtkSmartPointer<vtkPolyData> skeleton() const override
      { return m_data->skeleton(); }

      virtual void setSkeleton(vtkSmartPointer<vtkPolyData> skeleton) override
      { m_data->setSkeleton(skeleton); }

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
      { return m_data->fetchData(); }

    private:
      virtual QList<Data::Type> updateDependencies() const override
      { return m_data->dependencies(); }

    private:
      SkeletonDataSPtr m_data;

      friend class Output;
  };

  using SkeletonProxyPtr  = SkeletonProxy *;
  using SkeleotnProxySPtr = std::shared_ptr<SkeletonProxy>;

} // namespace ESPINA

#endif // ESPINA_SKELETON_DATA_PROXY_H_
