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
: m_mesh(nullptr)
{

}

//----------------------------------------------------------------------------
RawMesh::RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                 itkVolumeType::SpacingType spacing,
                 OutputSPtr output)
: m_mesh(mesh)
{

}

////----------------------------------------------------------------------------
//bool RawMesh::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
//{
//  bool dumped = false;
//
//  if (m_mesh)
//  {
//    vtkSmartPointer<vtkDoubleArray> spacingArray = vtkSmartPointer<vtkDoubleArray>::New();
//    spacingArray->SetName("Spacing");
//    spacingArray->SetNumberOfValues(3);
//    for (int i = 0; i < 3; ++i)
//      spacingArray->SetValue(i, m_spacing[i]);
//
//    m_mesh->GetPointData()->AddArray(spacingArray);
//
//    vtkSmartPointer<vtkGenericDataObjectWriter> polyWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
//    polyWriter->SetInputData(m_mesh);
//    polyWriter->SetFileTypeToBinary();
//    polyWriter->SetWriteToOutputString(true);
//    polyWriter->Write();
//
//    QByteArray polyArray(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
//
//    snapshot << SnapshotEntry(cachePath(prefix + "-AS.vtp"), polyArray);
//
//    dumped = true;
//  }
//
//  return dumped;
//}

using VTKReader = vtkSmartPointer<vtkGenericDataObjectReader>;

////----------------------------------------------------------------------------
//bool RawMesh::fetchSnapshot(Filter *filter, const QString &prefix)
//{
//  bool fetched = false;
//
//  QDir cacheDir = filter->cacheDir();
//
//  QString meshFileName = cachePath(QString("%1-AS.vtp").arg(prefix));
//  // Version 3 seg files compatibility
//  if (!cacheDir.exists(meshFileName))
//  {
//    meshFileName = QString("%1-AS.vtp").arg(prefix);
//  }
//
//  if (cacheDir.exists(meshFileName))
//  {
//    QString meshFile = cacheDir.absoluteFilePath(meshFileName);
//
//    VTKReader polyASReader = VTKReader::New();
//    polyASReader->SetFileName(meshFile.toUtf8());
//    polyASReader->SetReadAllFields(true);
//    polyASReader->Update();
//
//    m_mesh = vtkSmartPointer<vtkPolyData>(polyASReader->GetPolyDataOutput());
//
//    //vtkSmartPointer<vtkDoubleArray> spacingArray
//    vtkDataArray   *dataArray    = m_mesh->GetPointData()->GetArray("Spacing");
//    vtkDoubleArray *spacingArray = dynamic_cast<vtkDoubleArray *>(dataArray);
//    for (int i = 0; i < 3; ++i)
//      m_spacing[i] = spacingArray->GetValue(i);
//
//    fetched = true;
//  }

//   QString vectorName = QString().number(m_cacheId) + QString("-Vectors.dat");
// 
//   if (m_cacheDir.exists(vectorName))
//   {
//     QString fileName = m_cacheDir.absolutePath() + QDir::separator() + vectorName;
// 
//     QFile fileStream(fileName);
//     fileStream.open(QIODevice::ReadOnly | QIODevice::Text);
//     char buffer[1024];
//     while (fileStream.readLine(buffer, sizeof(buffer)) > 0)
//     {
//       QString line(buffer);
//       QStringList fields = line.split(SEP);
// 
//       if (fields[0] == QString("Template Origin"))
//       {
//         m_templateOrigin[0] = fields[1].toDouble();
//         m_templateOrigin[1] = fields[2].toDouble();
//         m_templateOrigin[2] = fields[3].toDouble();
//       }
// 
//       if (fields[0] == QString("Template Normal"))
//       {
//         m_templateNormal[0] = fields[1].toDouble();
//         m_templateNormal[1] = fields[2].toDouble();
//         m_templateNormal[2] = fields[3].toDouble();
//       }
//     }
//     returnValue = true;
//   }

//  return fetched;
//}

//----------------------------------------------------------------------------
bool RawMesh::isValid() const
{
  return true;
}

//----------------------------------------------------------------------------
bool RawMesh::setInternalData(MeshDataSPtr rhs)
{
  m_mesh = rhs->mesh();
  return true;
}

//----------------------------------------------------------------------------
bool RawMesh::isEdited() const
{
  return false;
}

//----------------------------------------------------------------------------
void RawMesh::clearEditedRegions()
{

}
//
////----------------------------------------------------------------------------
//void RawMesh::commitEditedRegions(bool withData) const
//{
//
//}
//
////----------------------------------------------------------------------------
//void RawMesh::restoreEditedRegions(const QDir &cacheDir, const QString &outputId)
//{
//
//}

//----------------------------------------------------------------------------
RawMeshSPtr EspINA::rawMesh(OutputSPtr output)
{
  RawMeshSPtr meshData = std::dynamic_pointer_cast<RawMeshSPtr>(output->data(RawMesh::TYPE));

  Q_ASSERT(mesh.get());
  return meshData;
}
