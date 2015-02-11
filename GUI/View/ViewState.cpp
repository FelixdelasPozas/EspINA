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

#include "ViewState.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
ViewState::ViewState()
: QObject()
, m_timeStamp{2}
{
}

//----------------------------------------------------------------------------
void ViewState::focusViewOn(const NmVector3 &point)
{
  setCrosshair(point);

  emit viewFocusedOn(point);
}

//----------------------------------------------------------------------------
void ViewState::setCrosshair(const NmVector3 &point)
{
  if (m_crosshair != point)
  {
    m_crosshair = point;

    ++m_timeStamp;

    emit crosshairChanged(point, m_timeStamp);
  }
}

//----------------------------------------------------------------------------
void ViewState::setCrosshairPlane(const Plane plane, const Nm position)
{
  NmVector3 crosshair = m_crosshair;

  crosshair[normalCoordinateIndex(plane)] = position;

  setCrosshair(crosshair);
}

//----------------------------------------------------------------------------
ESPINA::TimeStamp ViewState::timeStamp() const
{
  return m_timeStamp;
}

//----------------------------------------------------------------------------
ESPINA::NmVector3 ViewState::crosshair() const
{
  return m_crosshair;
}
