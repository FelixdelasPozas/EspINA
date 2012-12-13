/*
 * MeasureTool.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "MeasureTool.h"
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/MeasureWidget.h>

//----------------------------------------------------------------------------
MeasureTool::MeasureTool(ViewManager *vm)
: m_enabled(false)
, m_inUse(false)
, m_widget(NULL)
, m_viewManager(vm)
{
}

//----------------------------------------------------------------------------
MeasureTool::~MeasureTool()
{
  if(m_widget)
  {
    m_widget->setEnabled(false);
    delete m_widget;
    m_widget = NULL;
  }
}

//----------------------------------------------------------------------------
bool MeasureTool::filterEvent(QEvent *e, EspinaRenderView *view)
{
  if (m_inUse && m_enabled && m_widget)
    return m_widget->filterEvent(e, view);

  return false;
}

//----------------------------------------------------------------------------
void MeasureTool::setInUse(bool value)
{
  if(m_inUse == value)
    return;

  m_inUse = value;

  if (m_inUse)
  {
    m_widget = new MeasureWidget();
    m_viewManager->addWidget(m_widget);
    m_viewManager->setSelectionEnabled(false);
    m_widget->setEnabled(true);
  }
  else
  {
    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    m_viewManager->setSelectionEnabled(true);
    delete m_widget;
    m_widget = NULL;
  }
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
