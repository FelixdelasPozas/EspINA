/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/RepresentationPool.h>
#include "RepresentationUtils.h"

using namespace ESPINA;

const QString PLANE = "Plane";
const QString DEPTH = "Depth";

//--------------------------------------------------------------------
Plane GUI::RepresentationUtils::plane(const RepresentationState &state)
{
  return toPlane(state.getValue<int>(PLANE));
}

//--------------------------------------------------------------------
void GUI::RepresentationUtils::setPlane(RepresentationState &state, const Plane plane)
{
  state.setValue<int>(PLANE, idx(plane));
}

//--------------------------------------------------------------------
void GUI::RepresentationUtils::setPlane(RepresentationPoolSPtr pool, const Plane plane)
{
  pool->setSetting<int>(PLANE, idx(plane));
}

//--------------------------------------------------------------------
Nm GUI::RepresentationUtils::segmentationDepth(const RepresentationState &state)
{
  return state.getValue<Nm>(DEPTH);
}

//--------------------------------------------------------------------
void GUI::RepresentationUtils::setSegmentationDepth(RepresentationState &state, const Nm depth)
{
  state.setValue<Nm>(DEPTH, depth);
}

//--------------------------------------------------------------------
void GUI::RepresentationUtils::setSegmentationDepth(RepresentationPoolSPtr pool, const Nm depth)
{
  pool->setSetting<Nm>(DEPTH, depth);
}
