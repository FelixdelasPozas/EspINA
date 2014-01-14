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


#include "CircularBrushSelector.h"

#include <Filters/Utils/vtkTube.h>
#include <GUI/Selectors/BrushSelector.h>
#include <Core/Utils/Spatial.h>

#include <QDebug>
#include <QUndoStack>

using namespace EspINA;

//-----------------------------------------------------------------------------
CircularBrushSelector::CircularBrushSelector(ViewManagerSPtr vm, CategorySelector *categorySelector)
: BrushSelector(vm, categorySelector)
{
}


//-----------------------------------------------------------------------------
CircularBrushSelector::~CircularBrushSelector()
{

}

//-----------------------------------------------------------------------------
BrushSelector::BrushShape CircularBrushSelector::createBrushShape(ViewItemAdapterPtr item,
                                                                  NmVector3 center,
                                                                  Nm radius,
                                                                  Plane plane)
{
  double sRadius = (plane == Plane::YZ) ? 0 : radius;
  double cRadius = (plane == Plane::XZ) ? 0 : radius;
  double aRadius = (plane == Plane::XY) ? 0 : radius;

  Bounds brushBounds { center[0] - sRadius,
                       center[0] + sRadius,
                       center[1] - cRadius,
                       center[1] + cRadius,
                       center[2] - aRadius,
                       center[2] + aRadius };

  double baseCenter[3], topCenter[3];
  for (int i=0; i<3; i++)
    baseCenter[i] = topCenter[i] = center[i];

  int index = normalCoordinateIndex(plane);
  topCenter[index]  += 0.5 * m_spacing[index];
  baseCenter[index] -= 0.5 * m_spacing[index];

  vtkTube *brush = vtkTube::New();
  brush->SetBaseCenter(baseCenter);
  brush->SetBaseRadius(radius);
  brush->SetTopCenter(topCenter);
  brush->SetTopRadius(radius);

  return BrushShape(brush, brushBounds);
}