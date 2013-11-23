/*
 * ZoomTool.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */
#include "ZoomArea.h"

// EspINA

// Qt
#include <QPixmap>
#include <QAction>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomArea::ZoomArea(ViewManagerSPtr viewManager)
: m_enabled(false)
, m_widget(nullptr)
, m_viewManager(viewManager)
, m_zoomArea(new QAction(QIcon(":/espina/zoom_selection.png"), tr("Define Zoom Area"),this))
{
  QPixmap cursorBitmap;
  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);
  zoomCursor = QCursor(cursorBitmap, 0, 0);
}

//----------------------------------------------------------------------------
ZoomArea::~ZoomArea()
{
  if(m_widget)
  {
//     m_widget->setEnabled(false);
//     delete m_widget;
//     m_widget = nullptr;
  }
}

//----------------------------------------------------------------------------
void ZoomArea::setEnabled(bool value)
{

}

//----------------------------------------------------------------------------
bool ZoomArea::enabled() const
{

}

//----------------------------------------------------------------------------
QList< QAction* > ZoomArea::actions() const
{
  QList<QAction *> actions;

  actions << m_zoomArea;

  return actions;
}


// //----------------------------------------------------------------------------
// bool ZoomArea::filterEvent(QEvent *e, EspinaRenderView *view)
// {
//   if (m_inUse && m_enabled && m_widget)
//     return m_widget->filterEvent(e, view);
// 
//   return false;
// }
// 
// //----------------------------------------------------------------------------
// void ZoomArea::setInUse(bool value)
// {
//   if(m_inUse == value)
//     return;
// 
//   m_inUse = value;
// 
//   if (m_inUse)
//   {
//     m_widget = ZoomSelectionWidget::New();
//     m_viewManager->addWidget(m_widget);
//     m_viewManager->setSelectionEnabled(false);
//     m_widget->setEnabled(true);
//   }
//   else
//   {
//     m_widget->setEnabled(false);
//     m_viewManager->removeWidget(m_widget);
//     m_viewManager->setSelectionEnabled(true);
//     m_widget->Delete();
//     m_widget = NULL;
//   }
// }
// 
// //----------------------------------------------------------------------------
// void ZoomArea::setEnabled(bool value)
// {
//   m_enabled = value;
// }
// 
// //----------------------------------------------------------------------------
// bool ZoomArea::enabled() const
// {
//   return m_enabled;
// }
// 