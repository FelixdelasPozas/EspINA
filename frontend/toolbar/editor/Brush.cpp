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
#include "BrushUndoCommand.h"

#include "common/editor/BrushSelector.h"
#include "common/editor/FreeFormSource.h"
#include "common/gui/EspinaRenderView.h"
#include "common/gui/ViewManager.h"
#include "common/model/Channel.h"
#include "common/model/EspinaFactory.h"
#include "common/model/EspinaModel.h"
#include "common/model/Taxonomy.h"
#include "common/tools/BrushPicker.h"
#include <undo/AddSegmentation.h>

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
, m_inUse(false)
, m_mode(CREATE)
, m_erasing(false)
, m_brush(new BrushPicker())
, m_currentSource(NULL)
, m_currentSeg(NULL)
, m_currentOutput(NULL)
, m_drawCommand(NULL)
, m_eraseCommand(NULL)
{
  m_brush->setStrokeVisibility(false);//TODO 2012-10-26 Change once stroke preview is working
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
      m_brush->setStrokeVisibility(false);//TODO 2012-10-26 Change once stroke preview is working
      emit brushModeChanged(BRUSH);
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
      emit brushModeChanged(ERASER);
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
void Brush::setInUse(bool value)
{
  if (value == m_inUse)
    return;

  m_inUse = value;

  if (value && m_viewManager->activeTaxonomy() && m_viewManager->activeChannel())
  {
    m_brush->setBrushColor(m_viewManager->activeTaxonomy()->color());
    SegmentationList segs = selectedSegmentations();
    if (segs.size() == 1)
    {
      m_currentSeg = segs.first();
      m_currentSource = m_currentSeg->filter();
      m_currentOutput = m_currentSeg->outputNumber();

      m_brush->setBorderColor(QColor(Qt::green));
      m_brush->setReferenceItem(m_currentSeg);
    }
    else
    {
      m_currentSeg    = NULL;
      m_currentSource = NULL;
      m_currentOutput = -1;

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

//-----------------------------------------------------------------------------
void Brush::drawStroke(PickableItem* item,
                       IPicker::WorldRegion centers,
                       Nm radius,
                       PlaneType plane)
{
  if (centers->GetNumberOfPoints() == 0)
    return;

  if (m_erasing)
  {
    if (m_eraseCommand)
    {
      m_undoStack->push(m_eraseCommand);
      m_eraseCommand = NULL;
    }
  }else
  {
    if (m_drawCommand)
    {
      m_undoStack->push(m_drawCommand);
      m_drawCommand = NULL;
      m_brush->setBorderColor(QColor(Qt::green));
    }
//     BrushShapeList brushes;
// 
//     for (int i=0; i < centers->GetNumberOfPoints(); i++)
//       brushes << createBrushShape(item,
//                                   centers->GetPoint(i),
//                                   radius,
//                                   plane);
// 
//     if (!m_currentSource)
//     {
//       Q_ASSERT(!m_currentSeg);
// 
//       Q_ASSERT(ModelItem::CHANNEL == item->type());
// 
//       Channel *channel = dynamic_cast<Channel *>(item);
//       double spacing[3];
//       channel->spacing(spacing);
// 
//       Filter::NamedInputs inputs;
//       Filter::Arguments args;
//       FreeFormSource::Parameters params(args);
//       params.setSpacing(spacing);
//       m_currentSource = new FreeFormSource(inputs, args);
//       m_currentOutput = 0;
//       m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, m_currentOutput);
// 
//       m_undoStack->beginMacro("Draw Segmentation");
//       // We can't add empty segmentations to the model
//       m_undoStack->push(new DrawCommand(m_currentSource,
//                                         m_currentOutput,
//                                         brushes,
//                                         SEG_VOXEL_VALUE));
//       m_undoStack->push(new AddSegmentation(channel,
//                                             m_currentSource,
//                                             m_currentSeg,
//                                             m_viewManager->activeTaxonomy(),
//                                             m_model));
//       m_undoStack->endMacro();
//       m_brush->setBorderColor(QColor(Qt::green));
//     }else
//     {
//       Q_ASSERT(m_currentSource && m_currentSeg);
//       EspinaVolume::PixelType value = m_erasing?SEG_BG_VALUE:SEG_VOXEL_VALUE;
// 
//       m_undoStack->push(new DrawCommand(m_currentSource,
//                                         m_currentOutput,
//                                         brushes,
//                                         value));
//     }
  }

}

//-----------------------------------------------------------------------------
void Brush::drawStrokeStep(PickableItem* item,
                           double x, double y, double z,
                           Nm radius,
                           PlaneType plane)
{
  if (!m_erasing)
  {
    double center[3] = {x, y, z};
    BrushShape brush = createBrushShape(item, center, radius, plane);
    if (!m_drawCommand)
    {
      if (!m_currentSeg)
      {

        Q_ASSERT(ModelItem::CHANNEL == item->type());

        Channel *channel = dynamic_cast<Channel *>(item);
        double spacing[3];
        channel->spacing(spacing);

        Filter::NamedInputs inputs;
        Filter::Arguments args;
        FreeFormSource::Parameters params(args);
        params.setSpacing(spacing);
        m_currentSource = new FreeFormSource(inputs, args);
        m_currentOutput = 0;
        m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, m_currentOutput);
        m_currentSource->draw(m_currentOutput,
                              brush.first,
                              brush.second.bounds(),
                              SEG_VOXEL_VALUE);

        m_undoStack->beginMacro("Draw Segmentation");
        // We can't add empty segmentations to the model
        m_undoStack->push(new AddSegmentation(channel,
                                              m_currentSource,
                                              m_currentSeg,
                                              m_viewManager->activeTaxonomy(),
                                              m_model));
        m_undoStack->endMacro();
	m_drawCommand = new SnapshotCommand(m_currentSource,
                                          m_currentOutput);
      }
      else
      {
        m_drawCommand = new SnapshotCommand(m_currentSource,
                                            m_currentOutput);
        m_currentSource->draw(m_currentOutput,
                              brush.first,
                              brush.second.bounds(),
                              SEG_VOXEL_VALUE);
      }
    }
    else
    {
      Q_ASSERT(m_currentSeg);
      m_currentSource->draw(m_currentOutput,
                            brush.first,
                            brush.second.bounds(),
                            SEG_VOXEL_VALUE);
    }
    m_viewManager->updateViews();
  }
  else
  {
    Q_ASSERT(m_currentSeg);

    if (!m_eraseCommand)
    {
      m_eraseCommand = new SnapshotCommand(m_currentSource,
                                           m_currentOutput);
    }
    double center[3] = {x, y, z};
    BrushShape brush = createBrushShape(item, center, radius, plane);
    m_currentSource->draw(m_currentOutput,
                          brush.first,
                          brush.second.bounds(),
                          SEG_BG_VALUE);
    m_viewManager->updateViews();
  }
}
