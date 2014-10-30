/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "MeshTestingUtils.h"

#include <vtkCellArray.h>

vtkSmartPointer<vtkPolyData> ESPINA::Testing::createTriangleMesh()
{
  // Create a triangle
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint ( 1.0, 0.0, 0.0 );
  points->InsertNextPoint ( 0.0, 0.0, 0.0 );
  points->InsertNextPoint ( 0.0, 1.0, 0.0 );

  auto polys = vtkSmartPointer<vtkCellArray>::New();
  polys->InsertNextCell(3);
  polys->InsertCellPoint(0);
  polys->InsertCellPoint(1);
  polys->InsertCellPoint(2);

  // Create a polydata object
  auto polyData = vtkSmartPointer<vtkPolyData>::New();

  // Add the geometry and topology to the polydata
  polyData->SetPoints(points);
  polyData->SetPolys (polys );

  return polyData;
}

