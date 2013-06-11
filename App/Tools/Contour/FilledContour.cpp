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

#include "FilledContour.h"
#include <Undo/BrushUndoCommand.h>

// EspINA
#include <GUI/Pickers/ContourSelector.h>
#include <GUI/vtkWidgets/ContourWidget.h>
#include <GUI/ViewManager.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>
#include <Core/Model/ModelItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Taxonomy.h>
#include <Core/Filters/FreeFormSource.h>
#include <Undo/AddSegmentation.h>
#include <Undo/RemoveSegmentation.h>
#include <App/Undo/ContourUndoCommand.h>

// Qt
#include <QUndoStack>
#include <QtGui>

using namespace EspINA;

const Filter::FilterType FilledContour::FILTER_TYPE = "EditorToolBar::ContourSource";

//-----------------------------------------------------------------------------
FilledContour::FilledContour(EspinaModel *model,
                             QUndoStack  *undo,
                             ViewManager *viewManager)
: m_model(model)
, m_undoStack(undo)
, m_viewManager(viewManager)
, m_picker(new ContourSelector())
, m_enabled(false)
, m_inUse(false)
, m_contourWidget(NULL)
, m_widgetHasContour(false)
, m_lastContour(NULL)
{
  m_picker->setPickable(ISelector::CHANNEL);
}

//-----------------------------------------------------------------------------
FilledContour::~FilledContour()
{
  delete m_picker;
}

//-----------------------------------------------------------------------------
QCursor FilledContour::cursor() const
{
  return QCursor(Qt::CrossCursor);
}

//-----------------------------------------------------------------------------
bool FilledContour::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (!m_enabled || !m_contourWidget)
    return false;

  if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control)
    {
      m_contourWidget->setMode(Brush::BRUSH);
      m_viewManager->updateViews();
      emit changeMode(Brush::BRUSH);
      return true;
    }
  }
  else
    if (QEvent::KeyPress == e->type() && ((m_currentSeg && m_currentSource) || m_widgetHasContour))
    {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      if (ke->key() == Qt::Key_Control && ke->count() == 1)
      {
        m_contourWidget->setMode(Brush::ERASER);
        m_viewManager->updateViews();
        emit changeMode(Brush::ERASER);
        return true;
      }

      if (ke->key() == Qt::Key_Backspace)
        m_widgetHasContour = false;
    }

  return m_contourWidget->filterEvent(e, view);
}

//-----------------------------------------------------------------------------
void FilledContour::setInUse(bool enable)
{
  if(enable == m_inUse)
    return;

  m_inUse = enable;
  m_enabled = enable;
  emit changeMode(Brush::BRUSH);

  if (enable)
  {
    m_contourWidget = new ContourWidget();

    connect(m_contourWidget, SIGNAL(endContour()),
            this, SLOT(createUndoCommand()));
    connect(m_contourWidget, SIGNAL(rasterizeContours(ContourWidget::ContourList)),
            this, SLOT(rasterize(ContourWidget::ContourList)));

    SegmentationList selection;
    foreach(PickableItemPtr item, m_viewManager->selection())
    {
      if (EspINA::SEGMENTATION == item->type())
      selection << segmentationPtr(item);
    }

    if (selection.size() == 1)
    {
      m_currentSeg = m_model->findSegmentation(selection.first());
      m_currentSource = m_currentSeg->filter();
      m_contourWidget->setPolygonColor(m_viewManager->color(m_currentSeg.get()));
    }
    else
    {
      m_currentSeg.reset();
      m_currentSource.reset();
      m_contourWidget->setPolygonColor(m_viewManager->activeTaxonomy()->color());
    }

    m_viewManager->addWidget(m_contourWidget);
    m_viewManager->setSelectionEnabled(false);
    m_contourWidget->setEnabled(true);
  }
  else
  {
    if (m_widgetHasContour)
      rasterize(m_contourWidget->getContours());

    disconnect(m_contourWidget, SIGNAL(endContour()),
               this, SLOT(createUndoCommand()));
    disconnect(m_contourWidget, SIGNAL(rasterizeContours(ContourWidget::ContourList)),
               this, SLOT(rasterize(ContourWidget::ContourList)));

    m_viewManager->removeWidget(m_contourWidget);
    m_viewManager->setSelectionEnabled(true);
    m_viewManager->updateViews();
    m_contourWidget->setEnabled(false);
    delete m_contourWidget;
    m_contourWidget = NULL;
    m_currentSeg.reset();
    m_currentSource.reset();
  }
}

//-----------------------------------------------------------------------------
void FilledContour::setEnabled(bool enable)
{
  if(enable == m_enabled)
    return;

  m_enabled = enable;
}

//-----------------------------------------------------------------------------
bool FilledContour::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
void FilledContour::createUndoCommand()
{
  ChannelPtr channel = m_viewManager->activeChannel();

  if (!m_currentSeg)
  {
    m_undoStack->beginMacro("Draw segmentation using contours");
    m_undoStack->push(new ContourAddSegmentation(m_model->findChannel(channel),
                                                 m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()),
                                                 m_model,
                                                 m_viewManager,
                                                 this));
  }
  else
  {
    m_undoStack->beginMacro("Modify segmentation using contours");
    m_undoStack->push(new ContourUndoCommand(m_currentSeg, m_viewManager, this));
  }

  m_undoStack->endMacro();
  m_widgetHasContour = true;
}

//-----------------------------------------------------------------------------
void FilledContour::rasterize(ContourWidget::ContourList list)
{
  bool reduction = false;

  ContourWidget::ContourData contour;

  foreach(ContourWidget::ContourData contourElement, list)
    if (contourElement.PolyData != NULL)
    {
      contour = contourElement;
      if (contour.Mode == Brush::ERASER)
        reduction = true;

      break;
    }

  if (contour.PolyData == NULL || contour.PolyData->GetNumberOfPoints() == 0)
    return;

  Q_ASSERT(m_undoStack->index() >= 1);
  const QUndoCommand *command = m_undoStack->command(m_undoStack->index()-1);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (command->text() == QString("Draw segmentation using contours"))
  {
    const ContourAddSegmentation *addCommand = dynamic_cast<const ContourAddSegmentation *>(command->child(0));
    addCommand->rasterizeContour(contour);

    m_currentSeg = addCommand->getCreatedSegmentation();
  }
  else
    if (command->text() == QString("Modify segmentation using contours"))
    {
      const ContourUndoCommand *undoCommand = dynamic_cast<const ContourUndoCommand *>(command->child(0));
      undoCommand->rasterizeContour(contour);
    }
    else
      Q_ASSERT(false);

  if (reduction)
  {
    try
    {
      SegmentationVolumeSPtr segVolume = segmentationVolume(m_currentSeg->output());
      segVolume->fitToContent();
    }
    catch (...)
    {
      m_undoStack->push(new RemoveSegmentation(m_currentSeg.get(), m_model, m_viewManager));
      emit changeMode(Brush::BRUSH);
      m_currentSeg.reset();
      m_currentSource.reset();
      m_contourWidget->setPolygonColor(m_viewManager->activeTaxonomy()->color());
    }
  }

  if (m_currentSource)
    m_currentSource->notifyModification();

  QApplication::restoreOverrideCursor();
  m_widgetHasContour = false;
}

//-----------------------------------------------------------------------------
void FilledContour::setContour(ContourWidget::ContourData contour)
{
  if (!m_enabled)
    emit startDrawing();

  m_widgetHasContour = (contour.PolyData != NULL);
  m_contourWidget->initialize(contour);
}

//-----------------------------------------------------------------------------
ContourWidget::ContourData FilledContour::getContour()
{
  ContourWidget::ContourList list = m_contourWidget->getContours();

  foreach(ContourWidget::ContourData contour, list)
    if (contour.PolyData != NULL)
      return contour;

  return ContourWidget::ContourData();
}
