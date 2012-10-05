/*
 * ContourSelection.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#include "ContourSelector.h"
#include <selection/SelectableView.h>
#include <selection/SelectionManager.h>
#include <QPolygon>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVTKWidget.h>

ContourSelector::ContourSelector(SelectionHandler *succesor)
: SelectionHandler(succesor)
, m_cursor(Qt::CrossCursor)
{
}

ContourSelector::~ContourSelector()
{
}

bool ContourSelector::filterEvent(QEvent* e, SelectableView* view)
{
  switch (e->type())
  {
    case QEvent::Enter:
      return SelectionHandler::filterEvent(e, view);
      break;
    case QEvent::Leave:
      return SelectionHandler::filterEvent(e, view);
      break;
    case QEvent::Wheel:
      return SelectionHandler::filterEvent(e, view);
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
