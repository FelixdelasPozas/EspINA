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
RawMesh::RawMesh()
: m_mesh{nullptr}
{
}

//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 const NmVector3             &spacing,
                 const NmVector3             &origin)
: m_mesh(mesh)
{
  m_bounds = meshBounds(mesh, spacing, origin);
}


//----------------------------------------------------------------------------
void RawMesh::setSpacing(const NmVector3 &spacing)
{
  auto prevSpacing = m_bounds.spacing();
  if(m_mesh != nullptr)
  {
    Q_ASSERT(spacing[0] != 0 && spacing[1] != 0 && spacing[2] != 0);
    auto ratio = spacing / prevSpacing;

    PolyDataUtils::scalePolyData(m_mesh, ratio);
    updateModificationTime();
  }

  m_bounds = changeSpacing(m_bounds, spacing);
}

//----------------------------------------------------------------------------
void RawMesh::setMesh(vtkSmartPointer<vtkPolyData> mesh)
{
  bool existsMesh = (m_mesh != nullptr);

  if(!existsMesh)
  {
    m_mesh = vtkSmartPointer<vtkPolyData>::New();
  }

  m_mesh->DeepCopy(mesh);
  m_bounds = meshBounds(mesh, m_bounds.spacing(), m_bounds.origin());

  // only add as an edited region if there was a previous mesh.
  if (existsMesh)
  {
    BoundsList editedRegions;
    editedRegions << bounds();
    setEditedRegions(editedRegions);

    updateModificationTime();
  }
}

//----------------------------------------------------------------------------
bool RawMesh::isValid() const
{
  return m_bounds.areValid() && !needFetch();
}

//----------------------------------------------------------------------------
bool RawMesh::isEmpty() const
{
  return !m_mesh || m_mesh->GetNumberOfCells() == 0;
}

//----------------------------------------------------------------------------
size_t RawMesh::memoryUsage() const
{
  const int BYTES = 1024;
  return m_mesh ? m_mesh->GetActualMemorySize() * BYTES : 0;
}
