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

#ifndef ESPINA_RAW_MESH_H
#define ESPINA_RAW_MESH_H

#include "EspinaCore_Export.h"

#include <Core/Analysis/Data/MeshData.h>
#include "MeshProxy.h"

#include <vtkSmartPointer.h>

namespace EspINA
{
  class EspinaCore_EXPORT RawMesh
  : public MeshData
  {
  public:
    explicit RawMesh(OutputSPtr output = nullptr);
    explicit RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                     itkVolumeType::SpacingType spacing,
                     OutputSPtr output = nullptr);
    virtual ~RawMesh() {};

    virtual bool isValid() const;
    virtual bool setInternalData(MeshDataSPtr rhs);

    Snapshot snapshot() const
    { return Snapshot(); }

    virtual Snapshot editedRegionsSnapshot() const
    { return Snapshot(); }

    virtual Bounds bounds() const
    {
      Nm bounds[6];
      m_mesh->GetBounds(bounds);

      return Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
    }

    bool isEdited() const
    { return false; }

    void clearEditedRegions()
    { /* TODO: not allowed */ };

    virtual vtkSmartPointer<vtkPolyData> mesh() const;

    virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix);

    Snapshot snapshot(TemporalStorageSPtr storage, const QString &prefix) const;

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

  RawMeshSPtr EspinaCore_EXPORT rawMesh(OutputSPtr output);
}

#endif // ESPINA_RAW_MESH_H
