/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <App/ToolGroups/Visualize/FullscreenTool.h>
#include <App/EspinaMainWindow.h>

using namespace ESPINA;

const QString StateOnToltip = QString("Set application fullscreen off");
const QString StateOffTooltip = QString("Set application fullscreen on");

//--------------------------------------------------------------------
FullscreenTool::FullscreenTool(Support::Context &context, EspinaMainWindow &window)
: GenericTogglableTool{tr("FullscreenTool"), ":/espina/fullscreen.svg", StateOffTooltip, context}
, m_window            (window)
{
  setShortcut(Qt::Key_F11);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
}

//--------------------------------------------------------------------
void FullscreenTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);
}

//--------------------------------------------------------------------
void FullscreenTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}

//--------------------------------------------------------------------
void FullscreenTool::onToggled(bool value)
{
  if ((m_window.windowState() & Qt::WindowFullScreen))
  {
    setToolTip(StateOffTooltip);
  }
  else
  {
    setToolTip(StateOnToltip);
  }

  m_window.setWindowState(m_window.windowState() ^ Qt::WindowFullScreen);
}
