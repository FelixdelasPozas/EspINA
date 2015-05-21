/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "ZoomTool.h"
#include <GUI/View/Widgets/WidgetFactory.h>
#include <GUI/View/Widgets/Zoom/ZoomWidget2D.h>
#include <GUI/View/Widgets/Zoom/ZoomWidget3D.h>

using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::View::Widgets;

using namespace ESPINA;

//----------------------------------------------------------------------------
ZoomTool::ZoomTool(GUI::View::ViewState &viewState)
: m_viewState(viewState)
, m_action   {Tool::createAction(":/espina/zoom_selection.png", tr("Zoom Tool"), this)}
, m_handler  {new ZoomEventHandler()}
, m_factory  {new WidgetFactory{std::make_shared<ZoomWidget2D>(m_handler.get()), std::make_shared<ZoomWidget3D>(m_handler.get())}}
{
  m_action->setCheckable(true);

  connect(m_action, SIGNAL(toggled(bool)),
          this,     SLOT(onToolActivated(bool)));
}

//----------------------------------------------------------------------------
ZoomTool::~ZoomTool()
{
  disconnect(m_action, SIGNAL(toggled(bool)),
             this,     SLOT(onToolActivated(bool)));
}

//----------------------------------------------------------------------------
QList<QAction*> ZoomTool::actions() const
{
  QList<QAction *> actionList;

  actionList << m_action;

  return actionList;
}

//----------------------------------------------------------------------------
void ZoomTool::abortOperation()
{
  m_action->setChecked(false);
}

//----------------------------------------------------------------------------
void ZoomTool::onToolActivated(bool value)
{
  if (value)
  {
    m_viewState.setEventHandler(m_handler);
    m_viewState.addWidgets(m_factory);
  }
  else
  {
    m_viewState.removeWidgets(m_factory);
    m_viewState.unsetEventHandler(m_handler);
  }
}

//----------------------------------------------------------------------------
void ZoomTool::onToolEnabled(bool enabled)
{
}
