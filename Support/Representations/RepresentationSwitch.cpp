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

#include "RepresentationSwitch.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationSwitch::RepresentationSwitch(Timer &timer)
: m_timer(timer)
, m_active(false)
, m_settingsVisibility(false)
{

}

//-----------------------------------------------------------------------------
void RepresentationSwitch::setSettingsVisibility(bool visible)
{
  m_settingsVisibility = visible;

  onSettingsVisibilityChanged(visible);
}

//-----------------------------------------------------------------------------
void RepresentationSwitch::setActive(bool value)
{
  if (m_active != value)
  {
    m_active = value;

    auto t = m_timer.increment();

    if (m_active)
    {
      showRepresentations(t);
    }
    else
    {
      hideRepresentations(t);
    }
  }
}

//-----------------------------------------------------------------------------
void RepresentationSwitch::switchValue()
{
  setActive(!m_active);
}

//-----------------------------------------------------------------------------
bool RepresentationSwitch::isActive() const
{
  return m_active;
}
