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

#include "PanelSwitch.h"
#include "Panel.h"


using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
PanelSwitch::PanelSwitch(const QString &id, Panel *dock, const QString &icon, const QString &tooltip, Context &context)
: ProgressTool(id, icon, tooltip, context)
, m_dock(dock)
{
  Q_ASSERT(dock);

  setCheckable(true);
  setChecked(dock->isVisible());

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(showPanel(bool)));

  connect(dock, SIGNAL(dockShown(bool)),
          this, SLOT(setChecked(bool)));

  context.addPanel(dock);
}

//----------------------------------------------------------------------------
void PanelSwitch::showPanel(bool visible)
{
  m_dock->setVisible(visible);

  if (visible)
  {
    m_dock->raise();
  }
}

//----------------------------------------------------------------------------
void PanelSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}

//----------------------------------------------------------------------------
void PanelSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);
}

//----------------------------------------------------------------------------
void PanelSwitch::abortOperation()
{
  m_dock->abortOperation();
}
