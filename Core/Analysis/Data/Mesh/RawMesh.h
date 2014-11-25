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

#ifndef ESPINA_RAW_MESH_H
#define ESPINA_RAW_MESH_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data/MeshData.h>
#include "MeshProxy.h"

// VTK
#include <vtkSmartPointer.h>

namespace ESPINA
{
  class EspinaCore_EXPORT RawMesh
  : public MeshData
  {
  public:
    /** \brief RawMesh class constructor.
     * \param[in] output, smart pointer of associated output.
     *
     */
    explicit RawMesh(OutputSPtr output = nullptr);

    /** \brief RawMesh class constructor.
     * \param[in] mesh, vtkPolyData smart pointer.
     * \param[in] spacing, spacing of origin volume.
     * \param[in] output, smart pointer of associated output.
     *
     */
    explicit RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                     itkVolumeType::SpacingType spacing,
                     OutputSPtr output = nullptr);

    /** \brief RawMesh class virtual destructor.
     *
     */
    virtual ~RawMesh()
    {};

    virtual bool isValid() const
    { return (m_mesh.Get() != nullptr); }

    virtual bool isEmpty() const
    { return !isValid(); }

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const              override
    { return MeshData::snapshot(storage, path, id); }

    // Because meshes store the whole mesh polydata when their edited regions
    // are requested, we can use the same name which will cause fetch method to
    // succeed when restoring from edited regions (this will also will avoid
    // executing the filter itself if no other data is required)
    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override
    { return MeshData::snapshot(storage, path, id); };

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)            override
    { fetchDataImplementation(storage, path, id); }

    virtual vtkSmartPointer<vtkPolyData> mesh() const       override
    { return m_mesh; }

    virtual void setMesh(vtkSmartPointer<vtkPolyData> mesh) override;

    void setSpacing(const NmVector3&);

    NmVector3 spacing() const
    { return m_output->spacing(); }

    void undo()
    { /* TODO: not allowed */ };

    size_t memoryUsage() const;

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id) override
    { return MeshData::fetchDataImplementation(storage, path, id); }

  private:
    virtual QList<Data::Type> updateDependencies() const override
    { return QList<Data::Type>(); }

  private:
    QString snapshotFilename(const QString &path, const QString &id) const
    { return QString("%1/%2_%3.vtp").arg(path).arg(id).arg(type()); }

    QString oldSnapshotFilename(const QString &path, const QString &id) const
    { return QString("%1/%2_%3.vtp").arg(path).arg(type()).arg(id); }

    QString editedRegionSnapshotFilename(const QString &path, const QString &id) const
    { return snapshotFilename(path, id); }

  private:
    vtkSmartPointer<vtkPolyData> m_mesh;
  };

  using RawMeshPtr = RawMesh *;
  using RawMeshSPtr = std::shared_ptr<RawMesh>;

  /** \brief Obtains and returns the RawMesh smart pointer of the specified Output.
   * \param[in] output, Output object smart pointer.
   */
  RawMeshSPtr rawMesh(OutputSPtr output);
}

#endif // ESPINA_RAW_MESH_H
