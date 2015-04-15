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
#include "MeasureTool.h"
#include <GUI/View/Widgets/Measures/MeasureEventHandler.h>
#include <GUI/View/Widgets/Measures/MeasureWidget.h>

// Qt
#include <QAction>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Measures;


//----------------------------------------------------------------------------
MeasureTool::MeasureTool(ViewState & viewState)
: m_viewState{viewState}
, m_handler  {new MeasureEventHandler()}
, m_factory  {std::make_shared<WidgetFactory>(std::make_shared<MeasureWidget>(m_handler.get()), EspinaWidget3DSPtr())}
, m_action   {new QAction(QIcon(":/espina/measure.png"), tr("Segmentation Measures Tool"),this)}
{
  m_action->setCheckable(true);
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(initTool(bool)), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
MeasureTool::~MeasureTool()
{
}

//----------------------------------------------------------------------------
QList< QAction* > MeasureTool::actions() const
{
  QList<QAction *> actions;
  actions << m_action;

  return actions;
}

//----------------------------------------------------------------------------
void MeasureTool::abortOperation()
{
}

//----------------------------------------------------------------------------
void MeasureTool::initTool(bool value)
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

    emit stopMeasuring();
  }
}