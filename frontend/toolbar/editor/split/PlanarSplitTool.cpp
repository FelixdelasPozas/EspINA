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

// EspINA
#include "PlanarSplitTool.h"
#include "SplitUndoCommand.h"
#include "PlanarSplitWidget.h"
#include "common/gui/ViewManager.h"
#include "common/model/EspinaModel.h"
#include <editor/split/SplitFilter.h>
#include <gui/ViewManager.h>
#include <model/Segmentation.h>
#include <model/EspinaModel.h>
#include <model/EspinaFactory.h>
#include <undo/AddSegmentation.h>
#include <QUndoStack>

// Qt
#include <QtGlobal>
#include <QUndoStack>
#include <QApplication>

// vtk
#include <vtkPoints.h>

//-----------------------------------------------------------------------------
PlanarSplitTool::PlanarSplitTool(EspinaModel *model, QUndoStack *undo, ViewManager *vm)
: m_inUse(false)
, m_enabled(false)
, m_widget(NULL)
, m_model(model)
, m_undoStack(undo)
, m_viewManager(vm)
{
}

//-----------------------------------------------------------------------------
QCursor PlanarSplitTool::cursor() const
{
  return QCursor(Qt::CrossCursor);
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (m_inUse && m_enabled && m_widget)
    return m_widget->filterEvent(e, view);

  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setInUse(bool value)
{
  if (m_enabled != value)
    m_enabled = value;

  m_inUse = value;
  m_enabled = value;

  if (value)
  {
    m_widget = new PlanarSplitWidget();

    m_viewManager->addWidget(m_widget);
    m_viewManager->setSelectionEnabled(false);
    m_widget->setEnabled(true);
  }
  else
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    PlanarSplitWidget *widget = reinterpret_cast<PlanarSplitWidget*>(m_widget);
    vtkSmartPointer<vtkPoints> points = widget->getPlanePoints();

    if (points->GetNumberOfPoints() == 2)
    {
      // TODO: filtro y creación de segmentaciones

      // TODO: borrar esto, es sólo de control
      for (int i = 0; i < points->GetNumberOfPoints(); ++i)
      {
        double point[3];
        points->GetPoint(i, point);
        qDebug() << "point" << i << ":" << point[0] << point[1] << point[2];
      }
    }

    QApplication::restoreOverrideCursor();
    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    m_viewManager->setSelectionEnabled(true);
    delete m_widget;
  }
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setEnabled(bool value)
{
  m_enabled = value;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::splitSegmentation()
{
  Filter::NamedInputs inputs;
  Filter::Arguments   args;

  SegmentationList selectedSegs = m_viewManager->selectedSegmentations();
  Q_ASSERT(selectedSegs.size() == 1);

  Segmentation *seg = selectedSegs.first();

  inputs[SplitFilter::INPUTLINK] = seg->filter();
  args[Filter::INPUTS] = args.namedInput(SplitFilter::INPUTLINK, seg->outputNumber());

  SplitFilter *filter = new SplitFilter(inputs, args);
  filter->update();

  if (filter->numberOutputs() == 2)
    m_undoStack->push(new SplitUndoCommand(seg, filter, m_model));
  //else
    //report message
}
