/*
 * ZoomTool.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/ZoomTool.h"
#include "toolbar/zoom/ZoomSelectionWidget.h"
#include "common/gui/ViewManager.h"

//----------------------------------------------------------------------------
ZoomTool::ZoomTool(ViewManager* vm)
: m_enabled(false)
, m_inUse(false)
, m_widget(NULL)
, m_viewManager(vm)
{
}

//----------------------------------------------------------------------------
ZoomTool::~ZoomTool()
{
  if(m_widget)
  {
    m_widget->setEnabled(false);
    delete m_widget;
    m_widget = NULL;
  }
}

//----------------------------------------------------------------------------
QCursor ZoomTool::cursor() const
{
  return QCursor(Qt::ArrowCursor);
}

//----------------------------------------------------------------------------
bool ZoomTool::filterEvent(QEvent *e, EspinaRenderView *view)
{
  if (m_inUse && m_enabled && m_widget)
    return m_widget->filterEvent(e, view);

  return false;
}

//----------------------------------------------------------------------------
void ZoomTool::setInUse(bool value)
{
  if(m_inUse == value)
    return;

  m_inUse = value;

  if (m_inUse)
  {
    m_widget = new ZoomSelectionWidget();
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
void ZoomTool::setEnabled(bool value)
{
  m_enabled = value;
}

//----------------------------------------------------------------------------
bool ZoomTool::enabled() const
{
  return m_enabled;
}
