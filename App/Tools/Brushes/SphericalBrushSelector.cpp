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


#include "SphericalBrushSelector.h"

#include <vtkSphere.h>

#include <QDebug>
#include <QUndoStack>

using namespace EspINA;

//-----------------------------------------------------------------------------
SphericalBrushSelector::SphericalBrushSelector()
{
}

//-----------------------------------------------------------------------------
BrushSelector::BrushShape SphericalBrushSelector::createBrushShape(ViewItemAdapterPtr item,
                                                                   NmVector3          center,
                                                                   Nm                 radius,
                                                                   Plane              plane)
{
  Bounds brushBounds{ center[0] - radius,
                      center[0] + radius,
                      center[1] - radius,
                      center[1] + radius,
                      center[2] - radius,
                      center[2] + radius};

  double brushCenter[3]{ center[0], center[1], center[2] };
  vtkSphere *brush = vtkSphere::New();
  brush->SetCenter(brushCenter);
  brush->SetRadius(radius);

  return BrushShape(brush, brushBounds);
}
