/*
 * ZoomTool.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */
#include "ZoomTool.h"

// EspINA
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/ZoomSelectionWidget.h>

// Qt
#include <QPixmap>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomTool::ZoomTool(ViewManager* vm)
: m_enabled(false)
, m_inUse(false)
, m_widget(NULL)
, m_viewManager(vm)
{
  QPixmap cursorBitmap;
  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);
  this->zoomCursor = QCursor(cursorBitmap, 0, 0);
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
  return this->zoomCursor;
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
    m_widget = ZoomSelectionWidget::New();
    m_viewManager->addWidget(m_widget);
    m_viewManager->setSelectionEnabled(false);
    m_widget->setEnabled(true);
  }
  else
  {
    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    m_viewManager->setSelectionEnabled(true);
    m_widget->Delete();
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
