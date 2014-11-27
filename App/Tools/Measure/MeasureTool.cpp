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
#include <GUI/View/Widgets/Measures/MeasureWidget.h>

// Qt
#include <QAction>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  MeasureTool::MeasureTool(ViewManagerSPtr vm)
  : m_enabled    {false}
  , m_widget     {nullptr}
  , m_handler    {nullptr}
  , m_viewManager{vm}
  , m_action     {new QAction(QIcon(":/espina/measure.png"), tr("Segmentation Measures Tool"),this)}
  {
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered(bool)), this, SLOT(initTool(bool)), Qt::QueuedConnection);
  }

  //----------------------------------------------------------------------------
  MeasureTool::~MeasureTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      m_widget = nullptr;
    }
  }

  //----------------------------------------------------------------------------
  QList< QAction* > MeasureTool::actions() const
  {
    QList<QAction *> actions;
    actions << m_action;

    return actions;
  }

  //----------------------------------------------------------------------------
  void MeasureTool::setEnabled(bool value)
  {
    m_enabled = value;

    if (m_widget)
      m_widget->setEnabled(value);
  }

  //----------------------------------------------------------------------------
  bool MeasureTool::enabled() const
  {
    return m_enabled;
  }

  //----------------------------------------------------------------------------
  void MeasureTool::initTool(bool value)
  {
    if (value)
    {
      m_widget = EspinaWidgetSPtr(new MeasureWidget());
      m_handler = std::dynamic_pointer_cast<EventHandler>(m_widget);
      m_viewManager->setEventHandler(m_handler);
      m_viewManager->addWidget(m_widget);
      m_viewManager->setSelectionEnabled(false);
      m_widget->setEnabled(true);
    }
    else
    {
      m_widget->setEnabled(false);
      m_viewManager->removeWidget(m_widget);
      m_viewManager->unsetEventHandler(m_handler);
      m_handler = nullptr;
      m_viewManager->setSelectionEnabled(true);
      m_widget = nullptr;
      emit stopMeasuring();
    }
  }
}
