/*
 * MeasureTool.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "MeasureTool.h"
#include <GUI/View/Widgets/Measures/MeasureWidget.h>

// Qt
#include <QAction>

namespace EspINA
{
  //----------------------------------------------------------------------------
  MeasureTool::MeasureTool(ViewManagerSPtr vm)
  : m_enabled{false}
  , m_widget{nullptr}
  , m_handler{nullptr}
  , m_viewManager(vm)
  , m_action{new QAction(QIcon(":/espina/measure.png"), tr("Segmentation Measures Tool"),this)}
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
