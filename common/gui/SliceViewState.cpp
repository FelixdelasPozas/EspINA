/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

    This program is free software: you can redistribute it and/or modify
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


#include "SliceViewState.h"

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkProp3D.h>

#include <QDebug>

double AxialSliceMatrix[16] =
{
  1,  0,  0,  0,
  0,  1,  0,  0,
  0,  0,  1,  0,
  0,  0,  0,  1
};

//-----------------------------------------------------------------------------
void SliceView::AxialState::setCrossHairs(vtkPolyData* hline, vtkPolyData* vline, double center[3], double bounds[6])
{
  //   qDebug() << "Crosshair Center: " << center[0] << center[1] << center[2];
  hline->GetPoints()->SetPoint ( 0,bounds[0],center[1],0 );
  hline->GetPoints()->SetPoint ( 1,bounds[1],center[1],0 );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,center[0],bounds[2],0 );
  vline->GetPoints()->SetPoint ( 1,center[0],bounds[3],0 );
  vline->Modified();
}

//-----------------------------------------------------------------------------
void SliceView::AxialState::setSlicingPosition(vtkMatrix4x4* matrix, double value)
{
  matrix->SetElement(2, 3, value);
}

//-----------------------------------------------------------------------------
void SliceView::AxialState::updateCamera(vtkCamera* camera, double center[3])
{
  // and the award to the weirdest camera position goes to....
  camera->SetPosition(center[0], center[1], ((center[2] < 1) ? (center[2]-1) : (-center[2]+1)));
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void SliceView::AxialState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(AxialSliceMatrix);
}

//-----------------------------------------------------------------------------
double SagitalSliceMatrix[16] =
{
  0,  0, -1,  0,
  1,  0,  0,  0,
  0, -1,  0,  0,
  0,  0,  0,  1
};

//-----------------------------------------------------------------------------
void SliceView::SagittalState::setCrossHairs(vtkPolyData* hline,
                                             vtkPolyData* vline,
                                             double center[3],
                                             double bounds[6])
{
  hline->GetPoints()->SetPoint ( 0,0,center[1],bounds[4] );
  hline->GetPoints()->SetPoint ( 1,0,center[1],bounds[5] );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,0,bounds[2],center[2] );
  vline->GetPoints()->SetPoint ( 1,0,bounds[3],center[2] );
  vline->Modified();
}

//-----------------------------------------------------------------------------
void SliceView::SagittalState::setSlicingPosition(vtkMatrix4x4* matrix,
                                                double value)
{
  matrix->SetElement(0, 3, value);
}

//-----------------------------------------------------------------------------
void SliceView::SagittalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
  actor->RotateY(-90);
}

//-----------------------------------------------------------------------------
void SliceView::SagittalState::updateCamera(vtkCamera* camera,
                                            double center[3])
{
  camera->SetPosition(center[0] + 1,center[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void SliceView::SagittalState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(SagitalSliceMatrix);
}

//-----------------------------------------------------------------------------
double CoronalSliceMatrix[16] =
{
  1,  0,  0,  0,
  0,  0,  1,  0,
  0, -1,  0,  0,
  0,  0,  0,  1
};

//-----------------------------------------------------------------------------
void SliceView::CoronalState::setCrossHairs(vtkPolyData* hline,
                                            vtkPolyData* vline,
                                            double center[3],
                                            double bounds[6])
{
  hline->GetPoints()->SetPoint ( 0,bounds[0],0,center[2] );
  hline->GetPoints()->SetPoint ( 1,bounds[1],0,center[2] );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,center[0],0,bounds[4] );
  vline->GetPoints()->SetPoint ( 1,center[0],0,bounds[5] );
  vline->Modified();
}

//-----------------------------------------------------------------------------
void SliceView::CoronalState::setSlicingPosition(vtkMatrix4x4* matrix, double value)
{
  matrix->SetElement(1, 3, value);
}

//-----------------------------------------------------------------------------
void SliceView::CoronalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
}

//-----------------------------------------------------------------------------
void SliceView::CoronalState::updateCamera(vtkCamera* camera,
					 double center[3])
{
  camera->SetPosition(center[0], center[1]+1, center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetViewUp(0, 0, -1);
}

//-----------------------------------------------------------------------------
void SliceView::CoronalState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(CoronalSliceMatrix);
}
