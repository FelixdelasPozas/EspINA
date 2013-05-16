/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "SphericalBrush.h"

#include <vtkSphere.h>

#include <QDebug>
#include <QUndoStack>

using namespace EspINA;

//-----------------------------------------------------------------------------
SphericalBrush::SphericalBrush(EspinaModel *model,
                               QUndoStack  *undoStack,
                               ViewManager *viewManager)
: Brush(model, undoStack, viewManager)
{
}

//-----------------------------------------------------------------------------
Brush::BrushShape SphericalBrush::createBrushShape(PickableItemPtr item,
                                                   double          center[3],
                                                   Nm              radius,
                                                   PlaneType       plane)
{
  double brushBounds[6];
  brushBounds[0] = center[0] - radius;
  brushBounds[1] = center[0] + radius;
  brushBounds[2] = center[1] - radius;
  brushBounds[3] = center[1] + radius;
  brushBounds[4] = center[2] - radius;
  brushBounds[5] = center[2] + radius;

  vtkSphere *brush = vtkSphere::New();
  brush->SetCenter(center);
  brush->SetRadius(radius);

  return BrushShape(brush, EspinaRegion(brushBounds));
}
