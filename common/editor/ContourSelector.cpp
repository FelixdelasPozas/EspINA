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
  QMouseEvent *me = NULL;

  switch (e->type())
  {
    case QEvent::Enter:
      view->view()->grabKeyboard();
      return SelectionHandler::filterEvent(e, view);
      break;
    case QEvent::Leave:
      view->view()->releaseKeyboard();
      return SelectionHandler::filterEvent(e, view);
      break;
    default:
      return SelectionHandler::filterEvent(e, view);
      break;
  }

  // dead code, switch() handles all possibilities
  return true;
}

QCursor ContourSelector::cursor()
{
  return m_cursor;
}
