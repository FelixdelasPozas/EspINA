/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "BrushSelector.h"

#include <EspinaRenderView.h>
#include <vtkCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderWindow.h>
#include <QVTKWidget.h>

#include <QPixmap>
#include <QPaintEngine>
#include <QBitmap>
#include <QMouseEvent>
#include <QApplication>
#include <QEvent>

#include <QDebug>

//-----------------------------------------------------------------------------
BrushSelector::BrushSelector()
: m_state(DRAWING)
{
  setRadius(20);
}

//-----------------------------------------------------------------------------
//TODO: Pass only the QVTKWidget which contains the elements which we are interested in
bool BrushSelector::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      changeState(ERASING);
    view->setCursor(cursor());
    return true;
  }
  if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      changeState(DRAWING);
    view->setCursor(cursor());
    return true;
  }
  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta() / 8 / 15; //Refer to QWheelEvent doc.
      setRadius(m_radius + numSteps);
      view->setCursor(cursor());
      return true;
    }

    if (we->buttons() == Qt::LeftButton)
    {
      startSelection(we->x(), we->y(), view);
      return false;
    }
  }
  else if (QEvent::MouseButtonPress == e->type() || QEvent::MouseMove == e->type())
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);
    if (me->buttons() == Qt::LeftButton)
    {
      startSelection(me->x(), me->y(), view);
      return true;
    }
  }
  return IPicker::filterEvent(e, view);
}

//-----------------------------------------------------------------------------
QCursor BrushSelector::cursor()
{
  return m_cursor;
}

//-----------------------------------------------------------------------------
void BrushSelector::setRadius(int radius)
{
  static const int MAX_RADIUS = 32;
  if (radius > 0 && radius <= MAX_RADIUS)
  {
    m_radius = radius;
    int width = 2*radius;

    QPixmap pix(width, width);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QBrush(m_color));
    switch (m_state)
    {
      case CREATING:
        p.setPen(QPen(Qt::blue));
        break;
      case DRAWING:
        p.setPen(QPen(Qt::green));
        break;
      case ERASING:
        p.setPen(QPen(Qt::red));
        break;
    };

    p.drawEllipse(0, 0, width-1, width-1);
    Q_ASSERT(pix.hasAlpha());

    m_cursor = QCursor(pix);
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::startSelection(int x, int y, EspinaRenderView *view)
{
  DisplayRegionList regions;
  QPolygon brushRadius;

  int *winSize = view->renderWindow()->GetSize();
  int xPos, yPos;
  xPos = x;
  yPos = winSize[1] - y;

  brushRadius << QPoint(xPos,yPos)
              << QPoint(xPos+radius(), yPos)
              << QPoint(xPos, yPos+radius())
              << QPoint(xPos-radius(), yPos)
              << QPoint(xPos, yPos-radius());
  regions << brushRadius;

  PickList pickList = view->pick(m_filters, regions);

  emit itemsPicked(pickList);
}