/*
 * ContourSelection.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: Félix de las Pozas Alvarez
 */
#include "ContourPicker.h" //TODO 2012-27 Rename Class

#include <QPolygon>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVTKWidget.h>

ContourSelector::ContourSelector(IPicker *succesor)
: m_cursor(Qt::CrossCursor)
{
}

ContourSelector::~ContourSelector()
{
}

bool ContourSelector::filterEvent(QEvent* e, EspinaRenderView *view)
{
  switch (e->type())
  {
    case QEvent::Enter:
      return IPicker::filterEvent(e, view);
      break;
    case QEvent::Leave:
      return IPicker::filterEvent(e, view);
      break;
    case QEvent::Wheel:
      return IPicker::filterEvent(e, view);
      break;
    default:
      break;
  }

  return true;
}

QCursor ContourSelector::cursor()
{
  return m_cursor;
}
