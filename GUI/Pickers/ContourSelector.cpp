/*
 * ContourSelector.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: Félix de las Pozas Alvarez
 */
#include "ContourSelector.h"

#include <QPolygon>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVTKWidget.h>

using namespace EspINA;


ContourSelector::ContourSelector(ISelector *succesor)
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
      return ISelector::filterEvent(e, view);
      break;
    case QEvent::Leave:
      return ISelector::filterEvent(e, view);
      break;
    case QEvent::Wheel:
      return ISelector::filterEvent(e, view);
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
