/*
 * ContourSelector.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: Felix de las Pozas Alvarez
 */
#include "ContourSelector.h"

#include <QPolygon>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVTKWidget.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
bool ContourSelector::filterEvent(QEvent* e, RenderView *view)
{
  switch (e->type())
  {
    case QEvent::Enter:
      return Selector::filterEvent(e, view);
      break;
    case QEvent::Leave:
      return Selector::filterEvent(e, view);
      break;
    case QEvent::Wheel:
      return Selector::filterEvent(e, view);
      break;
    default:
      break;
  }

  return true;
}

//-----------------------------------------------------------------------------
QCursor ContourSelector::cursor()
{
  return m_cursor;
}
