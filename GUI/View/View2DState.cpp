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


#include "View2DState.h"

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkProp.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
void View2D::AxialState::setCrossHairs(vtkPolyData* hline,
                                          vtkPolyData* vline,
                                          const NmVector3& center,
                                          const Bounds&    bounds,
                                          const NmVector3& slicingStep)
{
  // the -0.1 is needed to draw the crosshair over the actors, right now the
  // segmentation's actors are been drawn -0.05 over the channel's actors at
  // the axial view
  Nm hShift = 0.5*slicingStep[0];
  Nm vShift = 0.5*slicingStep[1];

  hline->GetPoints()->SetPoint(0, bounds[0]-hShift, center[1], -0.1);
  hline->GetPoints()->SetPoint(1, bounds[1]+hShift, center[1], -0.1);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, center[0], bounds[2]-vShift, -0.1);
  vline->GetPoints()->SetPoint(1, center[0], bounds[3]+vShift, -0.1);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::AxialState::updateCamera(vtkCamera* camera, const NmVector3& center)
{
  double oldPos[3];
  camera->GetPosition(oldPos);
  oldPos[2] = (oldPos[2] < 1) ? oldPos[2] : -oldPos[2];

  // and the award to the weirdest camera position goes to....
  camera->SetPosition(center[0], center[1], oldPos[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void View2D::SagittalState::setCrossHairs(vtkPolyData* hline,
                                             vtkPolyData* vline,
                                             const NmVector3& center,
                                             const Bounds&    bounds,
                                             const NmVector3& slicingStep)
{
  // the +0.1 is needed to draw the crosshair over the actors, right now the
  // segmentation's actors are been drawn +0.05 over the channel's actors at
  // the sagittal view
  Nm hShift = 0.5*slicingStep[2];
  Nm vShift = 0.5*slicingStep[1];

  hline->GetPoints()->SetPoint(0, 0.1, center[1], bounds[4]-hShift);
  hline->GetPoints()->SetPoint(1, 0.1, center[1], bounds[5]+hShift);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, 0.1, bounds[2]-vShift, center[2]);
  vline->GetPoints()->SetPoint(1, 0.1, bounds[3]+vShift, center[2]);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::SagittalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
  actor->RotateY(-90);
}

//-----------------------------------------------------------------------------
void View2D::SagittalState::updateCamera(vtkCamera* camera,
                                            const NmVector3& center)
{
  camera->SetPosition(center[0] + 1,center[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void View2D::CoronalState::setCrossHairs(vtkPolyData*     hline,
                                            vtkPolyData*     vline,
                                            const NmVector3& center,
                                            const Bounds&    bounds,
                                            const NmVector3& slicingStep)
{
  // the +0.1 is needed to draw the crosshair over the actors, right now the
  // segmentation's actors are been drawn -0.05 over the channel's actors at
  // the coronal view
  Nm hShift = 0.5*slicingStep[0];
  Nm vShift = 0.5*slicingStep[2];

  hline->GetPoints()->SetPoint(0, bounds[0]-hShift, 0.1, center[2]);
  hline->GetPoints()->SetPoint(1, bounds[1]+hShift, 0.1, center[2]);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, center[0], 0.1, bounds[4]-vShift);
  vline->GetPoints()->SetPoint(1, center[0], 0.1, bounds[5]+vShift);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::CoronalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
}

//-----------------------------------------------------------------------------
void View2D::CoronalState::updateCamera(vtkCamera* camera,
                                           const NmVector3& center)
{
  camera->SetPosition(center[0], center[1]+1, center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetViewUp(0, 0, -1);
}