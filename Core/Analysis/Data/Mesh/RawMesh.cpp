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

// EspINA
#include "RawMesh.h"
#include <Core/Analysis/Filter.h>
#include <Core/Utils/vtkPolyDataUtils.h>

// VTK
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

// Qt
#include <QDir>

using namespace EspINA;

static QString MESHDATA_FILE = QString("MeshData_%1.vtp");

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
bool RawMesh::fetchData(const TemporalStorageSPtr storage, const QString& prefix)
{
  bool dataFetched = false;

  QString fileName = storage->absoluteFilePath(prefix + QString(MESHDATA_FILE).arg(m_output->id()));

  QFileInfo meshFile(fileName);

  if(meshFile.exists())
  {
    m_mesh = PolyDataUtils::readPolyDataFromFile(fileName);
    dataFetched = true;
  }

  return dataFetched;
}

//----------------------------------------------------------------------------
Snapshot RawMesh::snapshot(TemporalStorageSPtr storage, const QString &prefix) const
{
  QString fileName = prefix + QString(MESHDATA_FILE).arg(m_output->id());
  Snapshot snapshot;

  storage->makePath(prefix);

  if (m_mesh)
  {
    snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(m_mesh));
  }

  return snapshot;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> RawMesh::mesh() const
{
  if (m_mesh)
    return m_mesh;

  return nullptr;
}

//----------------------------------------------------------------------------
NmVector3 RawMesh::spacing() const
{
  return m_output->spacing();
}

//----------------------------------------------------------------------------
size_t RawMesh::memoryUsage() const
{
  if (m_mesh)
    return m_mesh->GetActualMemorySize();

  return 0;
}

//----------------------------------------------------------------------------
bool RawMesh::isValid() const
{
  return (m_mesh.Get() != nullptr);
}

//----------------------------------------------------------------------------
bool RawMesh::isEmpty() const
{
  return !isValid();
}

//----------------------------------------------------------------------------
bool RawMesh::setInternalData(MeshDataSPtr rhs)
{
  m_mesh = rhs->mesh();
  return true;
}


//----------------------------------------------------------------------------
RawMeshSPtr EspINA::rawMesh(OutputSPtr output)
{
  RawMeshSPtr meshData = std::dynamic_pointer_cast<RawMesh>(output->data(MeshData::TYPE));
  Q_ASSERT(meshData.get());
  return meshData;
}
