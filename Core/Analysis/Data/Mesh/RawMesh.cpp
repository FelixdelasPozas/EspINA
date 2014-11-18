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
bool RawMesh::fetchData(const TemporalStorageSPtr storage, const QString &path, const QString &id)
{
  bool dataFetched = false;

  // TODO: Fetch old file names
  // QString fileName = storage->absoluteFilePath(prefix + QString(MESHDATA_FILE).arg(m_output->id()));

  for (auto filename : {snapshotFilename(path, id),
                        oldSnapshotFilename(path, id)})
  {
    QFileInfo meshFile(storage->absoluteFilePath(filename));

    if(meshFile.exists())
    {
      m_mesh = PolyDataUtils::readPolyDataFromFile(meshFile.absoluteFilePath());
      dataFetched = true;
      break;
    }
  }

  return dataFetched;
}

//----------------------------------------------------------------------------
Snapshot RawMesh::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
{
  Snapshot snapshot;

  if (m_mesh)
  {
    QString fileName = snapshotFilename(path, id);
    storage->makePath(path);

    snapshot << SnapshotData(fileName, PolyDataUtils::savePolyDataToBuffer(m_mesh));
  }

  return snapshot;
}

//----------------------------------------------------------------------------
Snapshot RawMesh::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
{
  return snapshot(storage, path, id);
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> RawMesh::mesh() const
{
  return m_mesh;
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
RawMeshSPtr ESPINA::rawMesh(OutputSPtr output)
{
  RawMeshSPtr meshData = std::dynamic_pointer_cast<RawMesh>(output->data(MeshData::TYPE));
  Q_ASSERT(meshData.get());
  return meshData;
}
