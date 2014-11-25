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

// ESPINA
#include "RawMesh.h"
#include <Core/Analysis/Filter.h>
#include <Core/Utils/vtkPolyDataUtils.h>

// VTK
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

// Qt
#include <QDir>

using namespace ESPINA;

//----------------------------------------------------------------------------
RawMesh::RawMesh(OutputSPtr output)
: m_mesh{nullptr}
{
}

//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 itkVolumeType::SpacingType spacing,
                 OutputSPtr output)
: m_mesh{mesh}
{
}

//----------------------------------------------------------------------------
void RawMesh::setSpacing(const NmVector3 &newSpacing)
{
  if(m_mesh != nullptr)
  {
    auto oldSpacing = spacing();
    Q_ASSERT(newSpacing[0] != 0 && newSpacing[1] != 0 && newSpacing[2] != 0);
    NmVector3 ratio{newSpacing[0]/oldSpacing[0], newSpacing[1]/oldSpacing[1], newSpacing[2]/oldSpacing[2]};

    PolyDataUtils::scalePolyData(m_mesh, ratio);
    updateModificationTime();
  }
}

//----------------------------------------------------------------------------
void RawMesh::setMesh(vtkSmartPointer<vtkPolyData> mesh)
{
  m_mesh = mesh;

  BoundsList editedRegions;
  if (m_mesh)
  {
    editedRegions << bounds();
  }
  setEditedRegions(editedRegions);
}

//----------------------------------------------------------------------------
size_t RawMesh::memoryUsage() const
{
  if (m_mesh)
    return m_mesh->GetActualMemorySize();

  return 0;
}

//----------------------------------------------------------------------------
RawMeshSPtr ESPINA::rawMesh(OutputSPtr output)
{
  RawMeshSPtr meshData = std::dynamic_pointer_cast<RawMesh>(output->data(MeshData::TYPE));
  Q_ASSERT(meshData.get());
  return meshData;
}
