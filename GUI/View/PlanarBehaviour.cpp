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

//-----------------------------------------------------------------------------
void View2D::SagittalBehaviour::updateCamera(vtkCamera       *camera,
                                             const NmVector3 &center) const
{
  double *camPos = camera->GetPosition();
  if (camPos[0] == center[0])
  {
    ++camPos[0];
  }

  camera->SetPosition(camPos[0],center[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void View2D::CoronalBehaviour::updateCamera(vtkCamera       *camera,
                                            const NmVector3 &center) const
{
  double *camPos = camera->GetPosition();
  if (camPos[1] == center[1])
  {
    ++camPos[1];
  }

  camera->SetPosition(center[0], camPos[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetViewUp(0, 0, -1);
}

//-----------------------------------------------------------------------------
void View2D::AxialBehaviour::updateCamera(vtkCamera       *camera,
                                          const NmVector3 &center) const
{
  double oldPos[3];
  camera->GetPosition(oldPos);
  oldPos[2] = (oldPos[2] < 1) ? oldPos[2] : -oldPos[2];

  // and the award to the weirdest camera position goes to....
  camera->SetPosition(center[0], center[1], oldPos[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetRoll(180);
}
