/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#include "SkeletonTestingUtils.h"

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkIdList.h>

#include <cassert>
#include <random>

vtkSmartPointer<vtkPolyData> ESPINA::Testing::createRandomTestSkeleton(int numberOfNodes)
{
  assert(numberOfNodes >= 2);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 100);

  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(numberOfNodes);

  for(vtkIdType i = 0; i < numberOfNodes; ++i)
  {
    points->SetPoint(i, dis(gen), dis(gen), dis(gen));
  }

  auto lines = vtkSmartPointer<vtkCellArray>::New();

  for(vtkIdType i = 0; i < numberOfNodes -1; ++i)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i+1);
    lines->InsertNextCell(line);
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  return polyData;
}

vtkSmartPointer<vtkPolyData> ESPINA::Testing::createSimpleTestSkeleton()
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(4);
  points->SetPoint(0, 0.0, 0.0, 0.0);
  points->SetPoint(1, 1.0, 0.0, 0.0);
  points->SetPoint(2, 0.0, 1.0, 0.0);
  points->SetPoint(3, 0.0, 0.0, 1.0);

  auto lines = vtkSmartPointer<vtkCellArray>::New();
  for(vtkIdType i = 0; i < 3; ++i)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i+1);
    lines->InsertNextCell(line);
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  return polyData;
}
