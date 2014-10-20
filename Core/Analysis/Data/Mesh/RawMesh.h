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

    /** \brief Implements Data::isValid().
     *
     */
    virtual bool isValid() const;

    /** \brief Implements Data::isEmpty().
     *
     */
    virtual bool isEmpty() const;

    /** \brief Sets the data using a MeshData smart pointer.
     * \param[in] mesh, MeshData smart pointer.
     *
     */
    virtual bool setInternalData(MeshDataSPtr mesh);

    virtual bool fetchData(const TemporalStorageSPtr storage, const QString &path, const QString &id) override;

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override;

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override
    { return Snapshot(); }

    bool isEdited() const
    { return false; }

    void clearEditedRegions() override
    { /* TODO: not allowed */ };

    virtual vtkSmartPointer<vtkPolyData> mesh() const;

    void setSpacing(const NmVector3&)
    { /* TODO: not allowed */ };

    NmVector3 spacing() const;

    void undo()
    { /* TODO: not allowed */ };

    size_t memoryUsage() const;

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
