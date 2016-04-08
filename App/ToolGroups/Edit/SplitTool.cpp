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
#include <Core/Utils/EspinaException.h>
#include <Filters/SplitFilter.h>
#include <Filters/Utils/Stencil.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitEventHandler.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget2D.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget3D.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Widgets/Styles.h>
#include <Support/ContextFactories.h>
#include <Support/Settings/Settings.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>

// VTK
#include <vtkImageStencilData.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QToolButton>
#include <QMessageBox>
#include <QVBoxLayout>

using ESPINA::GUI::View::Widgets::PlanarSplitWidgetPtr;

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Filters::Utils;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::Support::ContextFactories;

const Filter::Type SplitFilterFactory::SPLIT_FILTER    = "SplitFilter";
const Filter::Type SplitFilterFactory::SPLIT_FILTER_V4 = "EditorToolBar::SplitFilter";

//-----------------------------------------------------------------------------
FilterTypeList SplitFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SPLIT_FILTER;
  filters << SPLIT_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SplitFilterFactory::createFilter(InputSList          inputs,
                                            const Filter::Type& type,
                                            SchedulerSPtr       scheduler) const
{
  if (!providedFilters().contains(type))
  {
    auto what    = QObject::tr("Unable to create filter: %1").arg(type);
    auto details = QObject::tr("SplitFilterFactory::createFilter() -> Unknown filter type: %1").arg(type);
    throw EspinaException(what, details);
  }

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
: EditTool("Split", ":/espina/planar_split.svg", tr("Split Segmentation"), context)
, m_handler{std::make_shared<PlanarSplitEventHandler>()}
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
                                                   std::make_shared<PlanarSplitWidget3D>(m_handler.get()),
                                                   id());

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
  m_apply = GUI::Widgets::Styles::createToolButton(":/espina/apply.svg", tr("Apply current state"));

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

  auto filter = getFactory()->createFilter<SplitFilter>(inputs, SplitFilterFactory::SPLIT_FILTER);
  filter->setDescription(tr("Split %1").arg(selectedSeg->data(Qt::DisplayRole).toString()));

  showTaskProgress(filter);

  auto output = selectedSeg->output();
  VolumeBounds bounds(output->bounds(), output->spacing());

  auto stencil = Stencil::fromPlane(m_splitPlane, bounds);
  filter->setStencil(stencil);

  Data data(filter, getModel()->smartPointer(selectedSeg));
  m_executingTasks.insert(filter.get(), data);

  connect(filter.get(), SIGNAL(finished()),
          this,         SLOT(createSegmentations()));

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
  WaitingCursor cursor;

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

      getSelection()->clear();
      getSelection()->set(toViewItemList(segmentations[1]));
    }
    else
    {
      auto msg = tr("Operation has NO effect. The defined plane does not split the selected segmentation into 2 segmentations.");
      
      DefaultDialogs::InformationMessage(msg);

      return;
    }
  }

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
  return EditTool::acceptsVolumetricSegmenations(segmentations);
}
