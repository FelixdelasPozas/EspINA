/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "ToggleSegmentationsVisibility.h"

#include <QAction>

using namespace EspINA;


//----------------------------------------------------------------------------
ToggleSegmentationsVisibility::ToggleSegmentationsVisibility(ViewManagerSPtr viewManager)
: m_viewManager(viewManager)
, m_toggle(QIcon(":/espina/show_all.svg"), tr("Toggle Segmentations visibility"), this)
{
  m_toggle.setCheckable(true);
  m_toggle.setChecked(true);
  m_toggle.setShortcut(QString("Space"));
  m_toggle.setShortcutContext(Qt::ApplicationShortcut);

  connect(&m_toggle, SIGNAL(triggered(bool)),
          this,      SLOT(toggleVisibility(bool)));
}


//----------------------------------------------------------------------------
QList<QAction *> ToggleSegmentationsVisibility::actions() const
{
  QList<QAction *> actions;

  actions << &m_toggle;

  return actions;
}

//----------------------------------------------------------------------------
bool ToggleSegmentationsVisibility::enabled() const
{
  return true;
}

//----------------------------------------------------------------------------
void ToggleSegmentationsVisibility::setEnabled(bool value)
{

}

//----------------------------------------------------------------------------
void ToggleSegmentationsVisibility::toggleVisibility(bool visible)
{
  QAction *action = dynamic_cast<QAction *>(sender());

  if (visible)
  {
    action->setIcon(QIcon(":/espina/show_all.svg"));
  } else
  {
    action->setIcon(QIcon(":/espina/hide_all.svg"));
  }

  m_viewManager->setSegmentationVisibility(visible);
}
