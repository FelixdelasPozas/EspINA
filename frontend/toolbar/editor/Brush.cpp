/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "Brush.h"

#include "common/editor/BrushSelector.h"
#include "common/gui/EspinaRenderView.h"
#include "common/gui/ViewManager.h"
#include "common/model/Taxonomy.h"
#include <model/Channel.h>
#include "common/tools/IPicker.h"
#include <vtkRenderWindow.h>

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

//-----------------------------------------------------------------------------
Brush::Brush(ViewManager *vm)
: m_viewManager(vm)
, m_mode(CREATE)
, m_erasing(false)
, m_tracking(false)
, m_radius(20)
{
}

//-----------------------------------------------------------------------------
Brush::~Brush()
{
}

//-----------------------------------------------------------------------------
QCursor Brush::cursor() const
{
  return m_cursor;
}

//-----------------------------------------------------------------------------
bool Brush::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (m_erasing)
  {
    if (e->type() == QEvent::KeyRelease)
    {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control)
      m_erasing = false;
    } else if (QEvent::MouseMove == e->type())
    {
      QMouseEvent *me = static_cast<QMouseEvent *>(e);
      if (Qt::CTRL != me->modifiers())
        m_erasing = false;
    }
    if (!m_erasing)
      buildCursor();
  } else
  {
    if (QEvent::KeyPress == e->type())
    {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      if (ke->key() == Qt::Key_Control && ke->count() == 1)
        m_erasing = true;
    } else if (QEvent::MouseMove == e->type())
    {
      QMouseEvent *me = static_cast<QMouseEvent *>(e);
      if (Qt::CTRL == me->modifiers())
        m_erasing = true;
    }
    if (m_erasing)
      buildCursor();
  }

  if (!m_tracking && QEvent::MouseButtonPress == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      m_tracking = true;
      m_dots << me->pos();
      return true;
    }
  } else if (m_tracking && QEvent::MouseMove == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    m_dots << me->pos();
    return true;
  }else if (m_tracking && QEvent::MouseButtonRelease == e->type())
  {
    processTrack(m_dots, view);

    m_tracking = false;
    m_dots.clear();
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void Brush::setEnabled(bool enable)
{
  if (enable)
    buildCursor();
  else
    emit stopDrawing();
}

//-----------------------------------------------------------------------------
void Brush::setInteraction(bool enable)
{
}

//-----------------------------------------------------------------------------
bool Brush::interactive() const
{
  return true;
}

//-----------------------------------------------------------------------------
void Brush::buildCursor()
{
  int width = 2*m_radius;

  QColor color(Qt::blue);

  if (m_viewManager->activeTaxonomy())
    color = m_viewManager->activeTaxonomy()->color();

  QPixmap pix(width, width);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setBrush(QBrush(color));

  if (m_erasing)
    p.setPen(Qt::red);
  else if (CREATE == m_mode)
      p.setPen(QPen(Qt::blue));
  else
      p.setPen(QPen(Qt::green));

  p.drawEllipse(0, 0, width-1, width-1);
  Q_ASSERT(pix.hasAlpha());

  m_cursor = QCursor(pix);
}

//-----------------------------------------------------------------------------
void Brush::processTrack(QList<QPoint> dots, EspinaRenderView *view)
{
  IPicker::DisplayRegionList displayBrushes;

  int *winSize = view->renderWindow()->GetSize();

  foreach(QPoint c, dots)
  {
    int xPos, yPos;
    xPos = c.x();
    yPos = winSize[1] - c.y();

    IPicker::DisplayRegion brushPolygon;
        brushPolygon << QPoint(xPos,yPos)
                     << QPoint(xPos+m_radius, yPos)
                     << QPoint(xPos, yPos+m_radius)
                     << QPoint(xPos-m_radius, yPos)
                     << QPoint(xPos, yPos-m_radius);
        displayBrushes << brushPolygon;
  }

  IPicker::PickableItems filter;
  filter << IPicker::CHANNEL;

  IPicker::PickList brushList = view->pick(filter, displayBrushes);

  Channel *channel = NULL;
  double spacing[3];
  PlaneType selectedPlane;
  Nm radius = -1;

  IPicker::WorldRegion trace;

  foreach(IPicker::PickedItem brush, brushList)
  {
    PickableItem *pickedItem = brush.second;
    Q_ASSERT(ModelItem::CHANNEL == pickedItem->type());
    if (!channel)
    {
      channel = dynamic_cast<Channel *>(pickedItem);
      channel->spacing(spacing);
    }

    IPicker::WorldRegion brushRegion = brush.first;
    if (brushRegion.size() != 5)
      continue;

    double points[5][3];
    double *center = points[0];
    double *right  = points[1];
    double *top    = points[2];
    //   double *left   = points[3];
    //   double *bottom = points[4];

    for (int i=0; i<5; i++)
    {
      points[i][0] = int(brushRegion[i].x()/spacing[0]+0.5)*spacing[0];
      points[i][1] = int(brushRegion[i].y()/spacing[1]+0.5)*spacing[1];
      points[i][2] = int(brushRegion[i].z()/spacing[2]+0.5)*spacing[2];
    }

    trace << brushRegion[0];

    if (radius <= 0)
    {
      if (center[0] == right[0] && right[0] == top[0])
        selectedPlane = SAGITTAL;
      else if (right[1] == center[1] && right[1] == top[1])
        selectedPlane = CORONAL;
      else if (center[2] == right[2] && right[2] == top[2])
        selectedPlane = AXIAL;

      double baseCenter[3], topCenter[3];
      for (int i=0; i<3; i++)
        baseCenter[i] = topCenter[i] = center[i];

      topCenter[selectedPlane] += 0.5*spacing[selectedPlane];

      if (selectedPlane != SAGITTAL)
        radius = fabs(brushRegion[0].x() - brushRegion[1].x());
      else
        radius = fabs(brushRegion[0].y() - brushRegion[2].y());
    }
  }

  if (m_erasing)
    emit eraserCenters(channel, trace, radius, selectedPlane);
  else
    emit brushCenters(channel, trace, radius, selectedPlane);
}
