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
  : public DataProxy
  {
    public:
      explicit MeshProxy() {}
      virtual ~MeshProxy() {}

      virtual void set(DataSPtr data)
      { m_data = std::dynamic_pointer_cast<MeshData>(data); }

      virtual DataSPtr get() const
      { return m_data; }

      /** \brief Return memory usage in MB
       *
       * Returns the amount of memory allocated to hold the volume representation
       */
      virtual double memoryUsage() const
      { return m_data->memoryUsage(); }

      virtual Bounds bounds() const
      { return m_data->bounds(); }

//      virtual void setOrigin(const typename T::PointType origin)
//      { m_data->setOrigin(origin); }
//
//      virtual typename T::PointType origin() const
//      { return m_data->origin(); }

      /** \brief Change every voxel value which satisfies the implicit function to the value given as parameter
       *
       *  If given bounds are not contained inside the volume bounds, the intersection will be applied
       */
//      virtual void draw(const vtkImplicitFunction* brush,
//                        const Bounds&      bounds,
//                        const typename T::ValueType value)
//      { m_data->draw(brush, bounds, value); }

//      virtual void draw(const itkImageSPtr volume,
//                        const Bounds&      bounds = Bounds())
//      { m_data->draw(volume, bounds); }

      /// Set voxels at index to value
      ///NOTE: Current implementation will expand the image
      ///      when drawing with value != 0
//      virtual void draw(itkVolumeType::IndexType index,
//                        itkVolumeType::PixelType value = SEG_VOXEL_VALUE)
//      { m_data->draw(index, value); }

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
      MeshDataSPtr m_data;

      friend class Output;
    };

    using MeshProxyPtr = MeshProxy *;
    using MeshProxySPtr = std::shared_ptr<MeshProxy>;


  } // namespace EspINA

//  class EspinaCore_EXPORT MeshProxy
//  : public DataProxy
//  {
//  public:
//    explicit MeshProxy(OutputSPtr output = nullptr);
//    virtual ~MeshProxy() {}
//
//    virtual bool setInternalData(MeshDataSPtr rhs);
//
//    virtual bool snapshot(const QString &prefix, Snapshot &snapshot) const;
//
//    virtual bool isValid() const;
//
//    virtual Bounds bounds();
//
//    virtual bool isEdited() const;
//
//    virtual void clearEditedRegions();
//
//    virtual void commitEditedRegions(bool withData) const;
//
//    virtual void restoreEditedRegions(const QDir &cacheDir, const QString &outputId);
//
//    virtual vtkAlgorithmOutput *mesh();
//
//  protected:
//    MeshDataSPtr m_meshData;
//  };
//
//  using MeshProxyPtr = MeshProxy *;
//  using MeshProxySPtr = std::shared<MeshProxy>;
//}

#endif // ESPINA_MESH_PROXY_H
