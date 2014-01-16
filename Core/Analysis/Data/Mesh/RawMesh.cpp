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

#include "RawMesh.h"
#include <Core/Analysis/Filter.h>

#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <QDir>

using namespace EspINA;

//----------------------------------------------------------------------------
RawMesh::RawMesh(OutputSPtr output)
: m_mesh{ vtkSmartPointer<vtkPolyData>::New() }
, m_spacing{ NmVector3{1,1,1} }
{
}

//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 itkVolumeType::SpacingType spacing,
                 OutputSPtr output)
: m_mesh{mesh}
, m_spacing{ NmVector3{spacing[0], spacing[1], spacing[2]} }
{
}

//----------------------------------------------------------------------------
bool RawMesh::fetchData(const TemporalStorageSPtr storage, const QString& prefix)
{
  bool dataFetched = false;
  int  error = 0;

  QString fileName = storage->absoluteFilePath(prefix + QString("MeshData.vtp"));

  QFileInfo meshFile(fileName);

  if(meshFile.exists())
  {
    vtkSmartPointer<vtkGenericDataObjectReader> reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->SetReadAllFields(true);
    reader->Update();

    error = reader->GetErrorCode();

    m_mesh = vtkSmartPointer<vtkPolyData>(reader->GetPolyDataOutput());

    vtkDataArray   *dataArray    = m_mesh->GetPointData()->GetArray("Spacing");
    vtkDoubleArray *spacingArray = dynamic_cast<vtkDoubleArray *>(dataArray);
    for (int i = 0; i < 3; ++i)
      m_spacing[i] = spacingArray->GetValue(i);

    dataFetched = true;
  }

  return dataFetched && (error != 0);
}

//----------------------------------------------------------------------------
Snapshot RawMesh::snapshot(TemporalStorageSPtr storage, const QString &prefix) const
{
  QString fileName = prefix + QString("MeshData.vtp");
  Snapshot snapshot;

  storage->makePath(prefix);

  if (m_mesh)
  {
    vtkSmartPointer<vtkDoubleArray> spacingArray = vtkSmartPointer<vtkDoubleArray>::New();
    spacingArray->SetName("Spacing");
    spacingArray->SetNumberOfValues(3);
    for (int i = 0; i < 3; ++i)
      spacingArray->SetValue(i, m_spacing[i]);

    m_mesh->GetPointData()->AddArray(spacingArray);

    vtkSmartPointer<vtkGenericDataObjectWriter> polyWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
    polyWriter->SetInputData(m_mesh);
    polyWriter->SetFileTypeToBinary();
    polyWriter->SetWriteToOutputString(true);
    polyWriter->Write();

    QByteArray polyArray(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());

    snapshot << SnapshotData(fileName, polyArray);
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
  if (m_mesh)
    return m_spacing;

  return NmVector3();
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
  return true;
}

//----------------------------------------------------------------------------
bool RawMesh::setInternalData(MeshDataSPtr rhs)
{
  m_mesh = rhs->mesh();
  m_spacing = rhs->spacing();
  return true;
}


//----------------------------------------------------------------------------
RawMeshSPtr EspINA::rawMesh(OutputSPtr output)
{
  RawMeshSPtr meshData = std::dynamic_pointer_cast<RawMesh>(output->data(RawMesh::TYPE));

  Q_ASSERT(meshData.get());
  return meshData;
}
