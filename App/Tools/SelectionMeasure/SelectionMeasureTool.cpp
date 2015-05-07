/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "SelectionMeasureTool.h"

#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/View/Widgets/SelectionMeasure/Widget2D.h>
#include <GUI/View/Widgets/SelectionMeasure/Widget3D.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QAction>

using namespace ESPINA;
using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;

//----------------------------------------------------------------------------
SelectionMeasureTool::SelectionMeasureTool(GUI::View::ViewState &viewState)
: m_viewState(viewState)
, m_factory  {new WidgetFactory(std::make_shared<Widget2D>(viewState.selection()), std::make_shared<Widget3D>(viewState.selection()))}
, m_action   {Tool::createAction(":/espina/measure3D.png", tr("Measure Selection"),this) }
{
  m_action->setCheckable(true);

  connect(m_action, SIGNAL(toggled(bool)),
          this,     SLOT(onToolActivated(bool)));
}

//----------------------------------------------------------------------------
SelectionMeasureTool::~SelectionMeasureTool()
{
}

//----------------------------------------------------------------------------
void SelectionMeasureTool::abortOperation()
{
  m_action->setChecked(false);
}

//----------------------------------------------------------------------------
QList<QAction*> SelectionMeasureTool::actions() const
{
  QList<QAction *> actionList;

  actionList << m_action;

  return actionList;
}

//----------------------------------------------------------------------------
void SelectionMeasureTool::onToolActivated(bool value)
{
  if (value)
  {
    m_viewState.addWidgets(m_factory);
  }
  else
  {
    m_viewState.removeWidgets(m_factory);
  }
}

//----------------------------------------------------------------------------
void SelectionMeasureTool::onToolEnabled(bool enabled)
{
}
