/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "RepresentationState.h"
#include <App/ToolGroups/1_Explore/Representations/RepresentationSettings.h>

using namespace ESPINA;
using namespace ESPINA::Representations;

//----------------------------------------------------------------------------
bool RepresentationState::hasValue(const QString &tag) const
{
  return m_properties.contains(tag);
}

//----------------------------------------------------------------------------
bool RepresentationState::hasPendingChanges() const
{
  for (auto pair : m_properties)
  {
    if (pair.second) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
void RepresentationState::apply(const RepresentationState &state)
{
  for (auto key : state.m_properties.keys())
  {
    auto pair    = state.m_properties[key];
    auto oldPair = m_properties.value(key, Pair(QVariant(), false));

    oldPair.second |= pair.first != oldPair.first;

    if (oldPair.second)
    {
       oldPair.first = pair.first;
       m_properties[key] = oldPair;
    }
  }
}

//----------------------------------------------------------------------------
void RepresentationState::commit()
{
  for (auto &pair : m_properties)
  {
    pair.second = false;
  }
}

//----------------------------------------------------------------------------
void RepresentationState::clear()
{
  m_properties.clear();
}

//----------------------------------------------------------------------------
bool ESPINA::hasCrosshairPoint(const RepresentationState &state)
{
  return state.hasValue(CROSSHAIR_X)
      && state.hasValue(CROSSHAIR_Y)
      && state.hasValue(CROSSHAIR_Z);
}

//----------------------------------------------------------------------------
void ESPINA::setCrosshairPoint(const NmVector3 &point, RepresentationState &state)
{
  state.setValue<double>(CROSSHAIR_X, point[0]);
  state.setValue<double>(CROSSHAIR_Y, point[1]);
  state.setValue<double>(CROSSHAIR_Z, point[2]);
}

//----------------------------------------------------------------------------
NmVector3 ESPINA::crosshairPoint(const RepresentationState &state)
{
  NmVector3 crosshair;

  crosshair[0] = state.getValue<double>(CROSSHAIR_X);
  crosshair[1] = state.getValue<double>(CROSSHAIR_Y);
  crosshair[2] = state.getValue<double>(CROSSHAIR_Z);

  return crosshair;
}

//----------------------------------------------------------------------------
Nm ESPINA::crosshairPosition(const Plane &plane, const RepresentationState &state)
{
  switch (plane)
  {
    case Plane::XY:
      return state.getValue<double>(CROSSHAIR_Z);
    case Plane::XZ:
      return state.getValue<double>(CROSSHAIR_Y);
    case Plane::YZ:
      return state.getValue<double>(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return 0;
}

//----------------------------------------------------------------------------
bool ESPINA::isCrosshairPointModified(const RepresentationState &state)
{
  return state.isModified(CROSSHAIR_X)
      || state.isModified(CROSSHAIR_Y)
      || state.isModified(CROSSHAIR_Z);
}

//----------------------------------------------------------------------------
bool ESPINA::isCrosshairPositionModified(const Plane &plane, const RepresentationState &state)
{
  switch (plane)
  {
    case Plane::XY:
      return state.isModified(CROSSHAIR_Z);
    case Plane::XZ:
      return state.isModified(CROSSHAIR_Y);
    case Plane::YZ:
      return state.isModified(CROSSHAIR_X);
    default:
      qWarning() << "Unexpected crosshair plane";
  }

  return false;
}

//----------------------------------------------------------------------------
bool ESPINA::isVisible(const RepresentationState &state)
{
  return state.getValue<bool>(VISIBLE);
}