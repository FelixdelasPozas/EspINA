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

    connect(m_contourWidget, SIGNAL(storeData()),
            this, SLOT(storeContourData()));
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

    disconnect(m_contourWidget, SIGNAL(storeData()),
               this, SLOT(storeContourData()));
    disconnect(m_contourWidget, SIGNAL(rasterizeContours(ContourWidget::ContourList)),
               this, SLOT(rasterize(ContourWidget::ContourList)));

    m_viewManager->removeWidget(m_contourWidget);
    m_viewManager->setSelectionEnabled(true);
    m_contourWidget->setEnabled(false);
    delete m_contourWidget;
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
void FilledContour::storeContourData()
{
  m_widgetHasContour = true;
}

//-----------------------------------------------------------------------------
void FilledContour::rasterize(ContourWidget::ContourList list)
{
  ChannelPtr channel = m_viewManager->activeChannel();
  bool reduction = false;

  if (list[AXIAL].Points == NULL && list[CORONAL].Points == NULL && list[SAGITTAL].Points == NULL)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!m_currentSeg)
    m_undoStack->beginMacro("Draw segmentation using contours");
  else
    m_undoStack->beginMacro("Modify segmentation using contours");

  foreach(ContourWidget::ContourData contour, list)
  {
    if (contour.Points == NULL)
      continue;

    if (contour.Mode == Brush::ERASER)
      reduction = true;

    if (!m_currentSource)
    {
      m_currentSource = FilterSPtr(new FreeFormSource(EspinaRegion(contour.Points->GetBounds()),
                                                      channel->volume()->spacing(),
                                                      FILTER_TYPE));
    }

    if (!m_currentSeg)
    {
      SegmentationVolumeSPtr currentVolume = segmentationVolume(m_currentSource->output(0));
      currentVolume->draw(contour.Points, contour.Points->GetPoint(0)[contour.Plane], contour.Plane, ((contour.Mode == Brush::BRUSH) ? SEG_VOXEL_VALUE : SEG_BG_VALUE), true);
      m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, 0);
      m_undoStack->push(new ContourAddSegmentation(m_model->findChannel(channel),
                                                   m_currentSource,
                                                   m_currentSeg,
                                                   m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()),
                                                   m_model,
                                                   this));
      SegmentationSList createdSegmentations;
      createdSegmentations << m_currentSeg;
      m_model->emitSegmentationAdded(createdSegmentations);

      ViewManager::Selection selectedSegmentations;
      selectedSegmentations << pickableItemPtr(m_currentSeg.get());
      m_viewManager->setSelection(selectedSegmentations);
    }
    else
    {
      m_undoStack->push(new ContourUndoCommand(m_currentSeg,
                                               contour.Points,
                                               contour.Points->GetPoint(0)[contour.Plane],
                                               contour.Plane,
                                               contour.Mode == Brush::BRUSH ? SEG_VOXEL_VALUE : SEG_BG_VALUE,
                                               m_viewManager,
                                               this));
    }
  }

  m_undoStack->endMacro();

  // if erasing must reduce the volume and check for complete deletion
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
void FilledContour::abortOperation()
{
  if (!m_enabled)
    return;

  if (m_widgetHasContour)
  {
    m_widgetHasContour = false;
    m_contourWidget->initialize();
  }

  emit stopDrawing();
}
