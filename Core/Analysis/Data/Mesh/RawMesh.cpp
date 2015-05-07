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
RawMesh::RawMesh(const NmVector3             &spacing,
                 const NmVector3             &origin)
: m_mesh(nullptr)
, m_spacing(spacing)
, m_origin(origin)
{
}
//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 const NmVector3             &spacing,
                 const NmVector3             &origin)
: m_mesh(mesh)
, m_spacing(spacing)
, m_origin(origin)
{
}


//----------------------------------------------------------------------------
void RawMesh::setSpacing(const NmVector3 &newSpacing)
{
  if(m_mesh != nullptr)
  {
    Q_ASSERT(newSpacing[0] != 0 && newSpacing[1] != 0 && newSpacing[2] != 0);
    NmVector3 ratio{newSpacing[0]/m_spacing[0],
                    newSpacing[1]/m_spacing[1],
                    newSpacing[2]/m_spacing[2]};

    PolyDataUtils::scalePolyData(m_mesh, ratio);
    updateModificationTime();
  }

  m_spacing = m_spacing;
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