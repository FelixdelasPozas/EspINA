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
#include <Undo/BrushUndoCommand.h>
#include <Undo/StrokeSegmentationCommand.h>
#include <Undo/VolumeSnapshotCommand.h>

#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Taxonomy.h>
#include <Core/EspinaSettings.h>
#include <GUI/Pickers/BrushPicker.h>
#include <GUI/QtWidget/EspinaRenderView.h>
#include <GUI/ViewManager.h>
#include <Filters/FreeFormSource.h>
#include <Undo/RemoveSegmentation.h>

#include <vtkRenderWindow.h>

#include <QUndoStack>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

using namespace EspINA;

const Filter::FilterType Brush::FREEFORM_SOURCE_TYPE = "EditorToolBar::FreeFormSource";

//-----------------------------------------------------------------------------
Brush::Brush(EspinaModel *model,
             QUndoStack  *undoStack,
             ViewManager *viewManager)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_inUse(false)
, m_mode(CREATE)
, m_erasing(false)
, m_brush(new BrushPicker())
, m_currentSource(NULL)
, m_currentSeg(NULL)
, m_currentOutput(-1)
, m_eraseCommand(NULL)
{
  connect(m_brush, SIGNAL(stroke(PickableItemPtr ,IPicker::WorldRegion, Nm, PlaneType)),
          this,  SLOT(drawStroke(PickableItemPtr ,IPicker::WorldRegion, Nm, PlaneType)));
  connect(m_brush, SIGNAL(stroke(PickableItemPtr,double,double,double,Nm,PlaneType)),
          this,  SLOT(drawStrokeStep(PickableItemPtr,double,double,double,Nm,PlaneType)));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(initBrushTool()));
}

//-----------------------------------------------------------------------------
Brush::~Brush()
{
  delete m_brush;
  if (m_eraseCommand)
    delete m_eraseCommand;
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
      {
        m_brush->DrawingOn(view);
        m_erasing = false;
        if (!m_currentSeg)
          m_brush->setBorderColor(QColor(Qt::blue));
      }
    }
    else
      if (QEvent::MouseMove == e->type())
      {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (Qt::CTRL != me->modifiers())
        {
          m_brush->DrawingOn(view);
          m_erasing = false;
          if (!m_currentSeg)
            m_brush->setBorderColor(QColor(Qt::blue));
        }
      }
    if (!m_erasing)
    {
      m_brush->setBorderColor(QColor(Qt::green));
      emit brushModeChanged(BRUSH);
    }
  }
  else
    if (m_currentSeg)
    {
      if (QEvent::KeyPress == e->type())
      {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Control && ke->count() == 1)
        {
          m_brush->DrawingOff(view);
          m_erasing = true;
        }
      }
      else
        if (QEvent::MouseMove == e->type())
        {
          QMouseEvent *me = static_cast<QMouseEvent *>(e);
          if (Qt::CTRL == me->modifiers())
          {
            m_brush->DrawingOff(view);
            m_erasing = true;
          }
        }

      if (m_erasing)
      {
        m_brush->setBorderColor(QColor(Qt::red));
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

  // we must avoid changing brush action until we have a segmentation created,
  // so avoid passing the CTRL key event to the brush
  if (QEvent::KeyPress == e->type() && !m_currentSeg)
    return false;

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
    initBrushTool();
  }
  else
  {
    if (m_currentSeg)
      disconnect(m_currentSeg.data(), SIGNAL(modified(ModelItemPtr)),
                 this, SLOT(segmentationHasBeenModified(ModelItemPtr)));

    emit stopDrawing();
  }
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
void Brush::drawStroke(PickableItemPtr item,
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
      m_undoStack->beginMacro("Erase Segmentation");
      m_undoStack->push(m_eraseCommand);
      m_viewManager->updateSegmentationRepresentations(m_currentSeg.data());
      try 
      {
        m_currentSeg->volume()->strechToFitContent();
      } catch (...)
      {
        m_undoStack->push(new RemoveSegmentation(m_currentSeg.data(), m_model));
        initBrushTool();
      }
      m_undoStack->endMacro();
      m_eraseCommand = NULL;
    }
  }
  else
  {
    BrushShapeList brushes;

    for (int i = 0; i < centers->GetNumberOfPoints(); i++)
      brushes << createBrushShape(item, centers->GetPoint(i), radius, plane);

    if (!m_currentSource)
    {
      Q_ASSERT(!m_currentSeg);

      Q_ASSERT(EspINA::CHANNEL == item->type());
      ChannelPtr channel = channelPtr(item);

      m_undoStack->beginMacro("Draw New Segmentation");
      {
        m_undoStack->push(new StrokeSegmentationCommand(channel,
                                                        m_viewManager->activeTaxonomy(),
                                                        brushes,
                                                        m_currentSeg,
                                                        m_model));
      }
      m_undoStack->endMacro();

      ViewManager::Selection selection;
      selection << m_currentSeg.data();
      m_viewManager->setSelection(selection);

      m_currentSource = m_currentSeg->filter();
      m_currentOutput = m_currentSeg->outputId();

      m_brush->setBorderColor(QColor(Qt::green));
      connect(m_currentSeg.data(), SIGNAL(modified(ModelItemPtr)),
              this, SLOT(segmentationHasBeenModified(ModelItemPtr)));
    }
    else
    {
      Q_ASSERT(m_currentSource && m_currentSeg);
      m_undoStack->beginMacro("Draw Segmentation");
      m_undoStack->push(new DrawCommand(m_currentSeg, brushes, SEG_VOXEL_VALUE, m_viewManager, this));
      m_undoStack->endMacro();
    }

    m_currentSeg->modifiedByUser(userName());
  }
}

//-----------------------------------------------------------------------------
void Brush::drawStrokeStep(PickableItemPtr item,
                           double x, double y, double z,
                           Nm radius,
                           PlaneType plane)
{
  if (m_erasing && m_currentSeg)
  {
    if (!m_eraseCommand)
    {
      m_eraseCommand = new VolumeSnapshotCommand(m_currentSeg->filter()->output(m_currentOutput));
    }
    double center[3] = { x, y, z };
    BrushShape brush = createBrushShape(item, center, radius, plane);
    m_currentSource->draw(m_currentOutput, brush.first, brush.second.bounds(), SEG_BG_VALUE);
    m_viewManager->updateSegmentationRepresentations(m_currentSeg.data());

    SegmentationList list;
    list.append(m_currentSeg.data());
    m_viewManager->forceRender(list);
  }
}

//-----------------------------------------------------------------------------
void Brush::segmentationHasBeenModified(ModelItemPtr item)
{
  Segmentation *seg = dynamic_cast<Segmentation *>(item);
  if (seg != m_currentSeg)
  {
    disconnect(seg, SIGNAL(modified(ModelItemPtr)), this, SLOT(segmentationHasBeenModified(ModelItemPtr)));
    return;
  }

  if (m_currentSeg->taxonomy()->color() != m_brush->getBrushColor())
  {
    m_brush->setBrushColor(m_currentSeg->taxonomy()->color());
    m_viewManager->updateViews();
  }
}

//-----------------------------------------------------------------------------
void Brush::initBrushTool()
{
  if (!m_inUse)
    return;

  if (m_currentSeg)
    disconnect(m_currentSeg.data(), SIGNAL(modified(ModelItemPtr)),
               this, SLOT(segmentationHasBeenModified(ModelItemPtr)));

  SegmentationList segs = m_viewManager->selectedSegmentations();
  if (segs.size() == 1)
  {
    m_currentSeg = m_model->findSegmentation(segs.first());
    connect(m_currentSeg.data(), SIGNAL(modified(ModelItemPtr)),
            this, SLOT(segmentationHasBeenModified(ModelItemPtr)));
    m_currentSource = m_currentSeg->filter();
    m_currentOutput = m_currentSeg->outputId();

    m_brush->setBrushColor(m_currentSeg->taxonomy()->color());
    m_brush->setBorderColor(QColor(Qt::green));
    m_brush->setReferenceItem(m_currentSeg.data());
  }
  else
  {
    m_currentSeg.clear();
    m_currentSource.clear();
    m_currentOutput = -1;

    m_brush->setBrushColor(m_viewManager->activeTaxonomy()->color());
    m_brush->setBorderColor(QColor(Qt::blue));
    m_brush->setReferenceItem(m_viewManager->activeChannel());
  }

  m_eraseCommand = NULL;
  m_erasing = false;
  m_brush->DrawingOn(NULL);
}
