/*
 * ZoomTool.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#include "ZoomArea.h"

// EspINA

// Qt
#include <QPixmap>
#include <QAction>
#include <GUI/View/Widgets/Zoom/ZoomSelectionWidget.h>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomArea::ZoomArea(ViewManagerSPtr viewManager)
: m_enabled{false}
, m_widget{nullptr}
, m_viewManager{viewManager}
, m_zoomArea(new QAction(QIcon(":/espina/zoom_selection.png"), tr("Define Zoom Area"),this))
, m_zoomHandler{nullptr}
{
  m_zoomArea->setCheckable(true);
  connect(m_zoomArea, SIGNAL(triggered(bool)), this, SLOT(initTool(bool)), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
ZoomArea::~ZoomArea()
{
  if(m_widget)
  {
     m_widget->setEnabled(false);
     m_widget = nullptr;
  }
}

//----------------------------------------------------------------------------
void ZoomArea::setEnabled(bool value)
{
  m_enabled = value;
}

//----------------------------------------------------------------------------
bool ZoomArea::enabled() const
{
  return m_enabled;
}

//----------------------------------------------------------------------------
QList< QAction* > ZoomArea::actions() const
{
  QList<QAction *> actions;
  actions << m_zoomArea;

  return actions;
}

//----------------------------------------------------------------------------
void ZoomArea::initTool(bool value)
{
  if (value)
  {
    m_widget = EspinaWidgetSPtr(ZoomSelectionWidget::New());
    m_zoomHandler = std::dynamic_pointer_cast<EventHandler>(m_widget);
    m_viewManager->setEventHandler(m_zoomHandler);
    m_viewManager->setSelectionEnabled(false);
    m_viewManager->addWidget(m_widget);
    m_widget->setEnabled(true);
  }
  else
  {
    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    m_viewManager->unsetEventHandler(m_zoomHandler);
    m_zoomHandler = nullptr;
    m_viewManager->setSelectionEnabled(true);
    m_widget = nullptr;
  }
}

//----------------------------------------------------------------------------
ZoomEventHandler::ZoomEventHandler(ZoomSelectionWidget *widget)
: m_widget{widget}
{
  QPixmap cursorBitmap;
  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);
  m_cursor = QCursor(cursorBitmap, 0, 0);
}

//----------------------------------------------------------------------------
bool ZoomEventHandler::filterEvent(QEvent *e, RenderView *view)
{
  if (m_inUse && m_widget)
    return m_widget->filterEvent(e, view);

  return false;
}

//----------------------------------------------------------------------------
void ZoomEventHandler::setInUse(bool value)
{
  if(m_inUse == value)
    return;

  m_inUse = value;

  emit eventHandlerInUse(value);
}
