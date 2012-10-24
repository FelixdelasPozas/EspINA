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
#include "common/model/Channel.h"
#include "common/tools/BrushPicker.h"

#include <vtkRenderWindow.h>

#include <QDebug>
#include <QUndoStack>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

//-----------------------------------------------------------------------------
Brush::Brush(EspinaModel* model,
             QUndoStack* undoStack,
             ViewManager* viewManager)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_mode(CREATE)
, m_erasing(false)
, m_brush(new BrushPicker())
, m_currentSource(NULL)
, m_currentSeg(NULL)
{
  connect(m_brush, SIGNAL(stroke(PickableItem *,IPicker::WorldRegion, Nm, PlaneType)),
          this,  SLOT(drawStroke(PickableItem *,IPicker::WorldRegion, Nm, PlaneType)));
  connect(m_brush, SIGNAL(stroke(PickableItem*,double,double,double,Nm,PlaneType)),
          this,  SLOT(drawStrokeStep(PickableItem*,double,double,double,Nm,PlaneType)));
}

//-----------------------------------------------------------------------------
Brush::~Brush()
{
}

//-----------------------------------------------------------------------------
QCursor Brush::cursor() const
{
  return m_brush->cursor();
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
    {
      m_brush->setBorderColor(QColor(Qt::green));
      m_brush->setStrokeVisibility(true);
    }
  } else if (m_currentSource)
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
    {
      m_brush->setBorderColor(QColor(Qt::red));
      m_brush->setStrokeVisibility(false);
    }
  }
  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta() / 8 / 15; //Refer to QWheelEvent doc.
      m_brush->setRadius(m_brush->radius() + numSteps);
      view->setCursor(cursor());
      return true;
    }
  }

  return m_brush->filterEvent(e, view);
}

//-----------------------------------------------------------------------------
void Brush::setInUse(bool enable)
{
  if (enable && m_viewManager->activeTaxonomy() && m_viewManager->activeChannel())
  {
    m_brush->setBrushColor(m_viewManager->activeTaxonomy()->color());
    SegmentationList segs = selectedSegmentations();
    if (segs.size() == 1)
    {
      m_currentSeg = segs.first();
      m_currentSource = m_currentSeg->filter();

      m_brush->setBorderColor(QColor(Qt::green));
      m_brush->setReferenceItem(m_currentSeg);
    }
    else
    {
      m_currentSeg = NULL;
      m_currentSource = NULL;

      m_brush->setBorderColor(QColor(Qt::blue));
      m_brush->setReferenceItem(m_viewManager->activeChannel());
    }
  }
  else
    emit stopDrawing();
}

//-----------------------------------------------------------------------------
void Brush::setEnabled(bool enable)
{
}

//-----------------------------------------------------------------------------
bool Brush::enabled() const
{
  return true;
}

//-----------------------------------------------------------------------------
SegmentationList Brush::selectedSegmentations() const
{
  SegmentationList selection;

  foreach(PickableItem *item, m_viewManager->selection())
  {
    if (ModelItem::SEGMENTATION == item->type())
      selection << dynamic_cast<Segmentation *>(item);
  }

  return selection;
}
