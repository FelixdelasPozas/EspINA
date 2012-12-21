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
#include <Tools/Brushes/Brush.h>
#include <Undo/BrushUndoCommand.h>

// EspINA
#include <GUI/Pickers/ContourPicker.h>
#include <GUI/vtkWidgets/ContourWidget.h>
#include <GUI/ViewManager.h>
#include <Core/Model/ModelItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Taxonomy.h>
#include <Filters/FreeFormSource.h>
#include <Filters/ContourSource.h>
#include <Undo/AddSegmentation.h>


#include <QUndoStack>
#include <QtGui>

using namespace EspINA;

//-----------------------------------------------------------------------------
FilledContour::FilledContour(EspinaModelSPtr model,
                             QUndoStack    *undo,
                             ViewManager   *vm)
: m_viewManager(vm)
, m_undoStack(undo)
, m_model(model)
, m_picker(new ContourSelector())
, m_enabled(false)
, m_inUse(false)
, m_contourWidget(NULL)
{
  m_picker->setPickable(IPicker::CHANNEL);
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

  return m_contourWidget->filterEvent(e, view);
}

//-----------------------------------------------------------------------------
void FilledContour::setInUse(bool enable)
{
  if(enable == m_inUse)
    return;

  m_inUse = enable;
  m_enabled = enable;

  if (enable)
  {
    m_contourWidget = new ContourWidget();

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
      m_contourWidget->setPolygonColor(m_viewManager->color(m_currentSeg.data()));
    }
    else
    {
      m_currentSeg.clear();
      m_currentSource.clear();
      m_contourWidget->setPolygonColor(m_viewManager->activeTaxonomy()->color());
    }

    m_viewManager->addWidget(m_contourWidget);
    m_contourWidget->setEnabled(true);
  }
  else
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (0 != m_contourWidget->GetContoursNumber())
    {
      ChannelPtr channel = m_viewManager->activeChannel();
      double spacing[3];
      channel->volume()->spacing(spacing);

      if (!m_currentSource && !m_currentSeg)
      {
        Filter::NamedInputs inputs;
        Filter::Arguments args;
        FreeFormSource::Parameters params(args);
        params.setSpacing(spacing);
        m_currentSource = FilterSPtr(new ContourSource(inputs, args));
      }

      if (!m_currentSeg && m_currentSource)
      {
        m_currentSource->draw(0, NULL, 0, AXIAL);
        m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, 0);
        m_undoStack->beginMacro("Draw segmentation using contours");
        m_undoStack->push(new AddSegmentation(m_model->findChannel(channel),
                                              m_currentSource,
                                              m_currentSeg,
                                              m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()),
                                              m_model));
        m_undoStack->endMacro();
      }
      else
      {
        m_undoStack->beginMacro("Modify segmentation using contours");
        m_undoStack->push(new Brush::SnapshotCommand(m_currentSource, 0));
        m_undoStack->endMacro();
      }

      QMap<PlaneType, QMap<Nm, vtkPolyData*> > contours = m_contourWidget->GetContours();
      QMap<Nm, vtkPolyData*>::iterator it = contours[AXIAL].begin();
      while (it != contours[AXIAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), AXIAL);
        ++it;
      }

      it = contours[CORONAL].begin();
      while (it != contours[CORONAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), CORONAL);
        ++it;
      }

      it = contours[SAGITTAL].begin();
      while (it != contours[SAGITTAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), SAGITTAL);
        ++it;
      }

      m_currentSource->notifyModification();
    }

    QApplication::restoreOverrideCursor();
    m_viewManager->removeWidget(m_contourWidget);
    m_contourWidget->setEnabled(false);
    delete m_contourWidget;
    m_currentSeg.clear();
    m_currentSource.clear();
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
