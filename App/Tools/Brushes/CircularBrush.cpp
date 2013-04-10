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


#include "CircularBrush.h"


#include "Core/VTK/vtkTube.h"

#include <QDebug>
#include <QUndoStack>

using namespace EspINA;

//-----------------------------------------------------------------------------
CircularBrush::CircularBrush(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *viewManager)
: Brush(model, undoStack, viewManager)
{
}


//-----------------------------------------------------------------------------
CircularBrush::~CircularBrush()
{

}

//-----------------------------------------------------------------------------
Brush::BrushShape CircularBrush::createBrushShape(PickableItemPtr item, double center[3], Nm radius, PlaneType plane)
{
  double spacing[3];
  item->volume()->spacing(spacing);

  double sRadius = (plane == SAGITTAL)?0:radius;
  double cRadius = (plane ==  CORONAL)?0:radius;
  double aRadius = (plane ==    AXIAL)?0:radius;

  double brushBounds[6];
  brushBounds[0] = center[0] - sRadius;
  brushBounds[1] = center[0] + sRadius;
  brushBounds[2] = center[1] - cRadius;
  brushBounds[3] = center[1] + cRadius;
  brushBounds[4] = center[2] - aRadius;
  brushBounds[5] = center[2] + aRadius;

  double baseCenter[3], topCenter[3];
  for (int i=0; i<3; i++)
    baseCenter[i] = topCenter[i] = center[i];
  topCenter[plane] += 0.5*spacing[plane];
  baseCenter[plane] -= 0.5*spacing[plane];

  vtkTube *brush = vtkTube::New();
  brush->SetBaseCenter(baseCenter);
  brush->SetBaseRadius(radius);
  brush->SetTopCenter(topCenter);
  brush->SetTopRadius(radius);

  return BrushShape(brush, EspinaRegion(brushBounds));
}
