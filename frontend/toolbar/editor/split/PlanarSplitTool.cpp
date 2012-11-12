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
#include "common/gui/EspinaRenderView.h"
#include <editor/split/SplitFilter.h>
#include <gui/ViewManager.h>
#include <model/Segmentation.h>
#include <model/EspinaModel.h>
#include <model/EspinaFactory.h>
#include <undo/AddSegmentation.h>
#include "common/EspinaRegions.h"
#include <QUndoStack>

// Qt
#include <QtGlobal>
#include <QUndoStack>
#include <QApplication>

// vtk
#include <vtkPoints.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkImageStencilData.h>
#include <vtkSphere.h>
#include <vtkPlane.h>

//-----------------------------------------------------------------------------
PlanarSplitTool::PlanarSplitTool(EspinaModel *model, QUndoStack *undo, ViewManager *vm)
: m_inUse(false)
, m_enabled(true)
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
  if (m_inUse == value)
    return;

  m_inUse = value;

  if (m_inUse)
  {
    m_widget = new PlanarSplitWidget();
    m_viewManager->addWidget(m_widget);
    m_viewManager->setSelectionEnabled(false);
    m_widget->setEnabled(true);

    SegmentationList selectedSegs = m_viewManager->selectedSegmentations();
    Q_ASSERT(selectedSegs.size() == 1);
    Segmentation *seg = selectedSegs.first();
    double bounds[6];
    VolumeBounds(seg->itkVolume(), bounds);
    EspinaVolume::SpacingType spacing = seg->itkVolume()->GetSpacing();
    bounds[0] -= 0.5*spacing[0];
    bounds[1] += 0.5*spacing[0];
    bounds[2] -= 0.5*spacing[1];
    bounds[3] += 0.5*spacing[1];
    bounds[4] -= 0.5*spacing[2];
    bounds[5] += 0.5*spacing[2];
    m_widget->setSegmentationBounds(bounds);
    m_viewManager->updateViews();
  }
  else
  {
    m_viewManager->setSelectionEnabled(true);

    if (m_widget->planeIsValid())
      splitSegmentation();

    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    delete m_widget;
    m_viewManager->updateViews();

    emit splittingStopped();
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
  QApplication::setOverrideCursor(Qt::WaitCursor);

  SegmentationList selectedSegs = m_viewManager->selectedSegmentations();
  Q_ASSERT(selectedSegs.size() == 1);

  Segmentation *seg = selectedSegs.first();

  EspinaVolume::PointType origin = seg->itkVolume()->GetOrigin();
  EspinaVolume::SpacingType spacing = seg->itkVolume()->GetSpacing();
  EspinaVolume::SizeType size = seg->itkVolume()->GetLargestPossibleRegion().GetSize();
  int segExtent[6];
  VolumeExtent(seg->itkVolume(), segExtent);

  vtkSmartPointer<vtkImplicitFunctionToImageStencil> plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
  plane2stencil->SetInput(m_widget->getImplicitPlane().GetPointer());
  plane2stencil->SetOutputOrigin(origin[0], origin[1], origin[2]);
  plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  //plane2stencil->SetOutputWholeExtent(origin[0], origin[0]+size[0], origin[1], origin[1]+size[1], origin[2], origin[2]+size[2]);
  plane2stencil->SetOutputWholeExtent(segExtent);
  plane2stencil->Update();

  Filter::NamedInputs inputs;
  Filter::Arguments   args;

  inputs[SplitFilter::INPUTLINK] = seg->filter();
  args[Filter::INPUTS] = args.namedInput(SplitFilter::INPUTLINK, seg->outputNumber());

  SplitFilter *filter = new SplitFilter(inputs, args);
  vtkSmartPointer<vtkImageStencilData> stencil = plane2stencil->GetOutput();
  filter->setStencil(stencil);
  filter->update();

  if (filter->numberOutputs() == 2)
  {
    Segmentation  *splitSeg[2];
    EspinaFactory *factory = m_model->factory();
    for (int i = 0; i < 2;  i++)
    {
      splitSeg[i] = factory->createSegmentation(filter, i);
      splitSeg[i]->setTaxonomy(seg->taxonomy());
    }

    if (filter->numberOutputs() == 2)
      m_undoStack->push(new SplitUndoCommand(seg, filter, splitSeg, m_model));
    //else
      //report message

    ViewManager::Selection selection;
    selection << splitSeg[0];
    m_viewManager->setSelection(selection);
  }
  QApplication::restoreOverrideCursor();
}
