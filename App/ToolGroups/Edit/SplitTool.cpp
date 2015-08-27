/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SplitTool.h"

#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/SplitFilter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitEventHandler.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget2D.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget3D.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/ContextFactories.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>

// VTK
#include <vtkImageStencilData.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>
#include <vtkPlane.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QToolButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>

using ESPINA::GUI::View::Widgets::PlanarSplitWidgetPtr;

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::Support::ContextFactories;

const Filter::Type SPLIT_FILTER    = "SplitFilter";
const Filter::Type SPLIT_FILTER_V4 = "EditorToolBar::SplitFilter";

//-----------------------------------------------------------------------------
FilterTypeList SplitTool::SplitFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SPLIT_FILTER;
  filters << SPLIT_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SplitTool::SplitFilterFactory::createFilter(InputSList          inputs,
                                                       const Filter::Type& type,
                                                       SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception)
{
  if (!providedFilters().contains(type)) throw Unknown_Filter_Exception();

  auto filter = std::make_shared<SplitFilter>(inputs, type, scheduler);

  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }

  filter->setDataFactory(m_dataFactory);

  return filter;
}

//------------------------------------------------------------------------
SplitTool::SplitTool(Support::Context &context)
: RefineTool("Split", ":/espina/planar_split.svg", tr("Split Segmentation"), context)
, m_handler{new PlanarSplitEventHandler()}
{
  registerFilterFactory(context, std::make_shared<SplitFilterFactory>());

  setCheckable(true);
  setExclusive(true);

  connect(this,  SIGNAL(toggled(bool)),
          this,  SLOT(toggleWidgetsVisibility(bool)));

  initSplitWidgets();

  connect(m_handler.get(), SIGNAL(widgetCreated(PlanarSplitWidgetPtr)),
          this,            SLOT(onWidgetCreated(PlanarSplitWidgetPtr)));

  connect(m_handler.get(), SIGNAL(widgetDestroyed(PlanarSplitWidgetPtr)),
          this,            SLOT(onWidgetDestroyed(PlanarSplitWidgetPtr)));

  connect(m_handler.get(), SIGNAL(planeDefined(PlanarSplitWidgetPtr)),
          this,            SLOT(onSplittingPlaneDefined(PlanarSplitWidgetPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(std::make_shared<PlanarSplitWidget2D>(m_handler.get()),
                                                   std::make_shared<PlanarSplitWidget3D>(m_handler.get()));

  setEventHandler(m_handler);
}

//------------------------------------------------------------------------
SplitTool::~SplitTool()
{
  disconnect(m_handler.get(), SIGNAL(widgetCreated(PlanarSplitWidgetPtr)),
             this,            SLOT(onWidgetCreated(PlanarSplitWidgetPtr)));

  disconnect(m_handler.get(), SIGNAL(widgetDestroyed(PlanarSplitWidgetPtr)),
             this,            SLOT(onWidgetDestroyed(PlanarSplitWidgetPtr)));

  disconnect(m_handler.get(), SIGNAL(planeDefined(PlanarSplitWidgetPtr)),
             this,            SLOT(onSplittingPlaneDefined(PlanarSplitWidgetPtr)));

  delete m_apply;
}

//-----------------------------------------------------------------------------
void SplitTool::initSplitWidgets()
{
  m_apply = GUI::Widgets::Styles::createToolButton(":/espina/tick.png", tr("Apply current state"));

  connect(m_apply, SIGNAL(clicked(bool)),
          this,  SLOT(applyCurrentState()));

  addSettingsWidget(m_apply);
}


//------------------------------------------------------------------------
void SplitTool::showCuttingPlane()
{
  auto selection = getSelection();

  connect(selection.get(), SIGNAL(selectionChanged()),
          this,            SLOT(onSelectionChanged()));

  getViewState().addTemporalRepresentations(m_factory);

  auto segmentation = getSelectedSegmentations().first();
  for(auto widget: m_splitWidgets)
  {
    widget->setSegmentationBounds(segmentation->bounds());
  }
}


//------------------------------------------------------------------------
void SplitTool::hideCuttingPlane()
{
  auto selection = getSelection();

  disconnect(selection.get(), SIGNAL(selectionChanged()),
             this,            SLOT(onSelectionChanged()));

  getViewState().removeTemporalRepresentations(m_factory);
}

//------------------------------------------------------------------------
void SplitTool::toggleWidgetsVisibility(bool visible)
{
  if (visible)
  {
    showCuttingPlane();
  }
  else
  {
    hideCuttingPlane();

    emit splittingStopped();
  }
}

//------------------------------------------------------------------------
void SplitTool::applyCurrentState()
{
  auto selectedSeg = getSelectedSegmentations().first();

  InputSList inputs;
  inputs << selectedSeg->asInput();

  auto filter = getFactory()->createFilter<SplitFilter>(inputs, SPLIT_FILTER);

  showTaskProgress(filter);

  auto spacing = selectedSeg->output()->spacing();
  auto bounds = selectedSeg->bounds();

  int extent[6]{ vtkMath::Round((bounds[0] + spacing[0] / 2) / spacing[0]),
                 vtkMath::Round((bounds[1] + spacing[0] / 2) / spacing[0]),
                 vtkMath::Round((bounds[2] + spacing[1] / 2) / spacing[1]),
                 vtkMath::Round((bounds[3] + spacing[1] / 2) / spacing[1]),
                 vtkMath::Round((bounds[4] + spacing[2] / 2) / spacing[2]),
                 vtkMath::Round((bounds[5] + spacing[2] / 2) / spacing[2]) };

  auto plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
  plane2stencil->SetInput(m_splitPlane);
  plane2stencil->SetOutputOrigin(0, 0, 0);
  plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  plane2stencil->SetOutputWholeExtent(extent);
  plane2stencil->Update();

  vtkSmartPointer<vtkImageStencilData> stencil = plane2stencil->GetOutput();
  filter->setStencil(stencil);

  Data data(filter, getModel()->smartPointer(selectedSeg));
  m_executingTasks.insert(filter.get(), data);

  connect(filter.get(), SIGNAL(finished()), this, SLOT(createSegmentations()));

  Task::submit(filter);
}

//------------------------------------------------------------------------
void SplitTool::onWidgetCreated(PlanarSplitWidgetPtr widget)
{
  Q_ASSERT(!m_splitWidgets.contains(widget));
  m_splitWidgets << widget;
}

//------------------------------------------------------------------------
void SplitTool::onWidgetDestroyed(PlanarSplitWidgetPtr widget)
{
  Q_ASSERT(m_splitWidgets.contains(widget));
  m_splitWidgets.removeOne(widget);
}

//------------------------------------------------------------------------
void SplitTool::onSplittingPlaneDefined(PlanarSplitWidgetPtr widget)
{
  auto valid = widget->planeIsValid();

  m_apply->setEnabled(valid);

  if(valid)
  {
    for(auto splitWidget: m_splitWidgets)
    {
      if(splitWidget == widget) continue;

      splitWidget->disableWidget();
    }

    auto spacing = getSelectedSegmentations().first()->output()->spacing();
    m_splitPlane = widget->getImplicitPlane(spacing);
  }
}

//------------------------------------------------------------------------
void SplitTool::onSelectionChanged()
{
  setChecked(false);
}

//------------------------------------------------------------------------
void SplitTool::createSegmentations()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  auto filter = dynamic_cast<FilterPtr>(sender());
  Q_ASSERT(m_executingTasks.keys().contains(filter));

  if(!filter->isAborted())
  {
    if (filter->numberOfOutputs() == 2)
    {
      auto sample = QueryAdapter::samples(m_executingTasks.value(filter).segmentation);
      auto category = m_executingTasks.value(filter).segmentation->category();

      SegmentationAdapterList  segmentations;
      SegmentationAdapterSList segmentationsList;

      for(auto i: {0, 1})
      {
        auto segmentation  = getFactory()->createSegmentation(m_executingTasks[filter].adapter, i);
        segmentation->setCategory(category);

        segmentationsList << segmentation;
        segmentations << segmentation.get();
      }

      auto undoStack = getUndoStack();
      undoStack->beginMacro("Split Segmentation");
      undoStack->push(new RemoveSegmentations(m_executingTasks[filter].segmentation.get(), getModel()));
      undoStack->push(new AddSegmentations(segmentationsList, sample, getModel()));
      undoStack->endMacro();

      deactivateEventHandler();

      getSelection()->set(toViewItemList(segmentations[1]));
    }
    else
    {
      QApplication::restoreOverrideCursor();
      QMessageBox warning;
      warning.setWindowModality(Qt::WindowModal);
      warning.setWindowTitle(tr("ESPINA"));
      warning.setIcon(QMessageBox::Warning);
      warning.setText(tr("Operation has NO effect. The defined plane does not split the selected segmentation into 2 segmentations."));
      warning.setStandardButtons(QMessageBox::Yes);
      warning.exec();
      return;
    }
  }

  QApplication::restoreOverrideCursor();
  m_executingTasks.remove(filter);
}

//------------------------------------------------------------------------
bool SplitTool::acceptsNInputs(int n) const
{
  return n > 0;
}

//------------------------------------------------------------------------
bool SplitTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return RefineTool::acceptsVolumetricSegmenations(segmentations);
}
