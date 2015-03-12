/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ToggleCrosshairVisibility.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
ToggleCrosshairVisibility::ToggleCrosshairVisibility(ViewManagerSPtr viewManager)
: m_viewManager{viewManager}
, m_toggle     {QIcon(":/espina/show_planes.svg"), tr("Toggle Crosshair visibility"), this}
{
  m_toggle.setCheckable(true);
  m_toggle.setChecked(false);

  connect(&m_toggle, SIGNAL(triggered(bool)),
          this,      SLOT(toggleVisibility(bool)));
}


//----------------------------------------------------------------------------
QList<QAction *> ToggleCrosshairVisibility::actions() const
{
  QList<QAction *> actions;

  actions << const_cast<QAction *>(&m_toggle);

  return actions;
}

//----------------------------------------------------------------------------
void ToggleCrosshairVisibility::toggleVisibility(bool visible)
{
  if (visible)
  {
    m_toggle.setIcon(QIcon(":/espina/show_planes.svg"));
  } else
  {
    m_toggle.setIcon(QIcon(":/espina/hide_planes.svg"));
  }

  m_viewManager->setCrosshairVisibility(visible);
}

//----------------------------------------------------------------------------
void ToggleCrosshairVisibility::onToolEnabled(bool enabled)
{
  m_toggle.setEnabled(enabled);
}

//----------------------------------------------------------------------------
void ToggleCrosshairVisibility::shortcut()
{
  toggleVisibility(!m_toggle.isChecked());
  m_toggle.setChecked(!m_toggle.isChecked());
}