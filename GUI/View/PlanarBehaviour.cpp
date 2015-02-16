/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// ESPINA
#include "PlanarBehaviour.h"

// VTK
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkProp.h>

// Qt
#include <QDebug>

using namespace ESPINA;

// the +0.1 is needed to draw the crosshair over the actors, right now the
// segmentation's actors are been drawn View2D::SEGMENTATION_SHIFT over the
// channel's actors at the view
const double ACTORS_SHIFT = 0.1;

//-----------------------------------------------------------------------------
void View2D::AxialBehaviour::setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                                       vtkSmartPointer<vtkPolyData> &vline,
                                       const NmVector3              &center,
                                       const Bounds                 &bounds,
                                       const NmVector3              &slicingStep)
{
  Nm hShift = 0.5*slicingStep[0];
  Nm vShift = 0.5*slicingStep[1];
  Nm zShift = bounds[4]-ACTORS_SHIFT;

  hline->GetPoints()->SetPoint(0, bounds[0]-hShift, center[1], zShift);
  hline->GetPoints()->SetPoint(1, bounds[1]+hShift, center[1], zShift);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, center[0], bounds[2]-vShift, zShift);
  vline->GetPoints()->SetPoint(1, center[0], bounds[3]+vShift, zShift);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::AxialBehaviour::updateCamera(vtkCamera       *camera,
                                      const NmVector3 &center)
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
void View2D::SagittalBehaviour::setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                                          vtkSmartPointer<vtkPolyData> &vline,
                                          const NmVector3              &center,
                                          const Bounds                 &bounds,
                                          const NmVector3              &slicingStep)
{
  Nm hShift = 0.5*slicingStep[2];
  Nm vShift = 0.5*slicingStep[1];
  Nm zShift = bounds[1]+ACTORS_SHIFT;

  hline->GetPoints()->SetPoint(0, zShift, center[1], bounds[4]-hShift);
  hline->GetPoints()->SetPoint(1, zShift, center[1], bounds[5]+hShift);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, zShift, bounds[2]-vShift, center[2]);
  vline->GetPoints()->SetPoint(1, zShift, bounds[3]+vShift, center[2]);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::SagittalBehaviour::updateCamera(vtkCamera       *camera,
                                         const NmVector3 &center)
{
  double *camPos = camera->GetPosition();
  if (camPos[0] == center[0])
    ++camPos[0];

  camera->SetPosition(camPos[0],center[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void View2D::CoronalBehaviour::setCrossHairs(vtkSmartPointer<vtkPolyData> &hline,
                                         vtkSmartPointer<vtkPolyData> &vline,
                                         const NmVector3              &center,
                                         const Bounds                 &bounds,
                                         const NmVector3              &slicingStep)
{
  Nm hShift = 0.5*slicingStep[0];
  Nm vShift = 0.5*slicingStep[2];
  Nm zShift = bounds[3]+ACTORS_SHIFT;

  hline->GetPoints()->SetPoint(0, bounds[0]-hShift, zShift, center[2]);
  hline->GetPoints()->SetPoint(1, bounds[1]+hShift, zShift, center[2]);
  hline->Modified();

  vline->GetPoints()->SetPoint(0, center[0], zShift, bounds[4]-vShift);
  vline->GetPoints()->SetPoint(1, center[0], zShift, bounds[5]+vShift);
  vline->Modified();
}

//-----------------------------------------------------------------------------
void View2D::CoronalBehaviour::updateCamera(vtkCamera       *camera,
                                        const NmVector3 &center)
{
  double *camPos = camera->GetPosition();
  if (camPos[1] == center[1])
    ++camPos[1];

  camera->SetPosition(center[0], camPos[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetViewUp(0, 0, -1);
}
