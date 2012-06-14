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


#include "PencilSelector.h"

#include <selection/SelectableView.h>
#include <EspinaCore.h>
#include <vtkCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderWindow.h>
#include <pqRenderViewBase.h>

#include <QPixmap>
#include <QPaintEngine>
#include <QBitmap>
#include <QMouseEvent>
#include <QApplication>
#include <QEvent>
#include <QWidget>

#include <QDebug>


//TODO: Generalize to Circular selector
//-----------------------------------------------------------------------------
PencilSelector::PencilSelector(SelectionHandler* succesor)
: SelectionHandler(succesor)
, m_tracking(false)
, m_state(DRAWING)
{
  setRadius(20);
}

//-----------------------------------------------------------------------------
bool PencilSelector::filterEvent(QEvent* e, SelectableView* view)
{
  if (e->type() == QEvent::Enter)
  {
    setRadius(m_radius);
    view->view()->getWidget()->setFocus();
    return SelectionHandler::filterEvent(e, view);
  } else if (e->type() == QEvent::KeyPress)
  {
   QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      changeState(ERASING);
    QApplication::changeOverrideCursor(cursor());
    return true;
  }
  if (e->type() == QEvent::KeyRelease)
  {
   QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      changeState(DRAWING);
    QApplication::changeOverrideCursor(cursor());
    return true;
  }
  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta()/8/15;//Refer to QWheelEvent doc.
      setRadius(m_radius+numSteps);
      QApplication::changeOverrideCursor(cursor());
      return true;
    }
  }else if (QEvent::MouseButtonPress == e->type()
    || QEvent::MouseMove == e->type())
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);
//     if (me->modifiers() == Qt::CTRL)
//       changeState(ERASING);
//     else
//       changeState(DRAWING);

    if (me->buttons() == Qt::LeftButton)
    {
      ViewRegions regions;
      QPolygon singlePixel;

      int *winSize = view->renderWindow()->GetSize();
      int xPos, yPos;
      xPos = me->x();
      yPos = winSize[1] - me->y();
//       view->eventPosition(xPos, yPos);
//       qDebug() << "Pick Pos" << xPos << yPos;
//       qDebug() << "Mouse Pos" << me->x() << me->y();
//       if (!m_tracking)
//       {
// 	m_xRef = me->x();
// 	m_yRef = -me->y();
// 	//       qDebug() << "Reference" << m_xRef << m_yRef;
//       }
//       m_tracking = true;
//       //     qDebug() << "Mouse" << me->x() << me->y();
//       xPos = xPos + (me->x() - m_xRef);
//       yPos = yPos + (-me->y() - m_yRef);
//       //     qDebug() << "Final" << xPos << yPos;

      singlePixel << QPoint(xPos,yPos) << QPoint(xPos+radius(), yPos);
      regions << singlePixel;

      MultiSelection msel = view->select(m_filters, regions);

      emit selectionChanged(msel);
      return true;
    }
  }
  else {
    m_tracking = false;
  }
  return SelectionHandler::filterEvent(e, view);
}

//-----------------------------------------------------------------------------
QCursor PencilSelector::cursor()
{
  return m_cursor;
}

//-----------------------------------------------------------------------------
void PencilSelector::setRadius(int radius)
{
  static const int MAX_RADIUS = 32;
  if (radius > 0 && radius <= MAX_RADIUS)
  {
    m_radius = radius;
    int width = 2*radius;

    QPixmap pix(width, width);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    if (EspinaCore::instance()->activeTaxonomy())
    {
      QColor taxColor = EspinaCore::instance()->activeTaxonomy()->color();
//       p.setPen(QPen(taxColor));
      taxColor.setAlphaF(0.8);
      p.setBrush(QBrush(taxColor));
    }
    else
    {
      p.setBrush(QBrush(QColor(0,255,0,120)));
//       p.setPen(QPen(QColor(0,255,0,255)));
    }
    if (m_state == DRAWING)
      p.setPen(QPen(Qt::green));
    else
      p.setPen(QPen(Qt::red));
    p.drawEllipse(0, 0, width-1, width-1);
    Q_ASSERT(pix.hasAlpha());

    m_cursor = QCursor(pix);
  }
}

