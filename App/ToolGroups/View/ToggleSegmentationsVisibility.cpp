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
#include "ToggleSegmentationsVisibility.h"

// Qt
#include <QAction>

using namespace ESPINA;

//----------------------------------------------------------------------------
ToggleSegmentationsVisibility::ToggleSegmentationsVisibility(ViewManagerSPtr viewManager)
: m_viewManager{viewManager}
, m_toggle     {QIcon(":/espina/show_all.svg"), tr("Toggle Segmentations visibility"), this}
{
  m_toggle.setCheckable(true);
  m_toggle.setChecked(true);

  connect(&m_toggle, SIGNAL(triggered(bool)),
          this,      SLOT(toggleVisibility(bool)));
}


//----------------------------------------------------------------------------
QList<QAction *> ToggleSegmentationsVisibility::actions() const
{
  QList<QAction *> actions;

  actions << const_cast<QAction *>(&m_toggle);

  return actions;
}

//----------------------------------------------------------------------------
bool ToggleSegmentationsVisibility::enabled() const
{
  return m_toggle.isEnabled();
}

//----------------------------------------------------------------------------
void ToggleSegmentationsVisibility::setEnabled(bool value)
{
  m_toggle.setEnabled(value);
}

//----------------------------------------------------------------------------
void ToggleSegmentationsVisibility::toggleVisibility(bool visible)
{
  if (visible)
  {
    m_toggle.setIcon(QIcon(":/espina/show_all.svg"));
  }
  else
  {
    m_toggle.setIcon(QIcon(":/espina/hide_all.svg"));
  }
}

//----------------------------------------------------------------------------
void ToggleSegmentationsVisibility::shortcut()
{
  toggleVisibility(!m_toggle.isChecked());
  m_toggle.setChecked(!m_toggle.isChecked());
}
