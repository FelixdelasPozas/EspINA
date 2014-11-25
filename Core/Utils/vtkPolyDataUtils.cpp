/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// VTK
#include "vtkPolyDataUtils.h"
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkPolyData.h>

// QT
#include <QByteArray>
#include <QString>

using namespace ESPINA;

//------------------------------------------------------------------------------------
QByteArray ESPINA::PolyDataUtils::savePolyDataToBuffer(const vtkSmartPointer<vtkPolyData> polyData)
throw (IO_Error_Exception)
{
  vtkSmartPointer<vtkGenericDataObjectWriter> polyWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
  polyWriter->SetInputData(polyData);
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetWriteToOutputString(true);
  polyWriter->Write();

  if (polyWriter->GetErrorCode() != 0)
    throw IO_Error_Exception();

  return QByteArray(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
}

//------------------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> ESPINA::PolyDataUtils::readPolyDataFromFile(QString fileName)
throw (IO_Error_Exception)
{
  vtkSmartPointer<vtkGenericDataObjectReader> reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
  reader->SetFileName(fileName.toUtf8());
  reader->SetReadAllFields(true);
  reader->Update();

  if (reader->GetErrorCode() != 0)
    throw IO_Error_Exception();

  vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
  mesh->DeepCopy(reader->GetPolyDataOutput());

  return mesh;
}

//------------------------------------------------------------------------------------
void EspinaCore_EXPORT ESPINA::PolyDataUtils::scalePolyData(vtkSmartPointer<vtkPolyData> polyData, const NmVector3 &ratio)
{
  auto points = polyData->GetPoints();
  double point[3];

  for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    points->GetPoint(i, point);
    for(auto coord: {0,1,2})
    {
      point[coord] = point[coord] * ratio[coord];
    }
    points->SetPoint(i, point);
  }

  points->Modified();
  polyData->Modified();
}
