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
#include <GUI/Model/Utils/ModelUtils.h>
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
#include <vtkPlane.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QToolButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Filters::Utils;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::Support::ContextFactories;

const Filter::Type SplitFilterFactory::SPLIT_FILTER    = "SplitFilter";
const Filter::Type SplitFilterFactory::SPLIT_FILTER_V4 = "EditorToolBar::SplitFilter";

//-----------------------------------------------------------------------------
const FilterTypeList SplitFilterFactory::providedFilters() const
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
, m_splitPlane{nullptr}
{
  registerFilterFactory(context, std::make_shared<SplitFilterFactory>());

  setCheckable(true);
  setExclusive(true);

  connect(this,  SIGNAL(toggled(bool)),
          this,  SLOT(toggleWidgetsVisibility(bool)));

  initSplitWidgets();

  connect(m_handler.get(), SIGNAL(planeDefined(GUI::View::Widgets::PlanarSplitWidgetPtr)),
          this,            SLOT(onSplittingPlaneDefined(GUI::View::Widgets::PlanarSplitWidgetPtr)));

  auto factory2D = std::make_shared<PlanarSplitWidget2D>(m_handler.get());
  auto factory3D = std::make_shared<PlanarSplitWidget3D>(m_handler.get());

  connect(factory2D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,            SLOT(onWidgetCreated(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  connect(factory3D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation3DSPtr)),
          this,            SLOT(onWidgetCreated(GUI::Representations::Managers::TemporalRepresentation3DSPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(factory2D, factory3D, id());

  setEventHandler(m_handler);
}

//------------------------------------------------------------------------
SplitTool::~SplitTool()
{
  if(isChecked()) setChecked(false);

  disconnect(m_handler.get(), SIGNAL(planeDefined(GUI::View::Widgets::PlanarSplitWidgetPtr)),
             this,            SLOT(onSplittingPlaneDefined(GUI::View::Widgets::PlanarSplitWidgetPtr)));

  disconnect(this,  SIGNAL(toggled(bool)),
             this,  SLOT(toggleWidgetsVisibility(bool)));

  disconnect(m_apply, SIGNAL(clicked(bool)),
             this,  SLOT(applyCurrentState()));

  abortTasks();
}

//-----------------------------------------------------------------------------
void SplitTool::initSplitWidgets()
{
  m_apply = GUI::Widgets::Styles::createToolButton(":/espina/apply.svg", tr("A cutting plane must be defined in 2D or 3D widgets before it can to applied."));

  connect(m_apply, SIGNAL(clicked(bool)),
          this,  SLOT(applyCurrentState()));

  addSettingsWidget(m_apply);

  // not enabled until a valid cutting plane has been defined.
  m_apply->setEnabled(false);
}

//------------------------------------------------------------------------
void SplitTool::showCuttingPlane()
{
  auto selection = getSelection();

  connect(selection.get(), SIGNAL(selectionChanged()),
          this,            SLOT(onSelectionChanged()));

  getViewState().addTemporalRepresentations(m_factory);

  auto segmentation = getSelectedSegmentations().first();

  for(auto obj: m_splitWidgets)
  {
    auto widget = dynamic_cast<PlanarSplitWidgetPtr>(obj);
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

    m_splitPlane = nullptr;

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

  selectedSeg->setBeingModified(true);

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
void SplitTool::onWidgetCreated(TemporalRepresentation2DSPtr widget)
{
  auto splitWidget2D = std::dynamic_pointer_cast<PlanarSplitWidget2D>(widget);

  auto obj = dynamic_cast<QObject *>(splitWidget2D.get());
  Q_ASSERT(!m_splitWidgets.contains(obj));
  connect(obj,  SIGNAL(destroyed(QObject *)),
          this, SLOT(onWidgetDestroyed(QObject *)), Qt::DirectConnection);

  m_splitWidgets << obj;

  if(m_splitPlane != nullptr)
  {
    splitWidget2D->disableWidget();
  }
  else
  {
    auto segmentation = getSelectedSegmentations().first();
    splitWidget2D->setSegmentationBounds(segmentation->bounds());
  }
}

//------------------------------------------------------------------------
void SplitTool::onWidgetCreated(TemporalRepresentation3DSPtr widget)
{
  auto splitWidget3D = std::dynamic_pointer_cast<PlanarSplitWidget3D>(widget);

  auto obj = dynamic_cast<QObject *>(splitWidget3D.get());
  Q_ASSERT(!m_splitWidgets.contains(obj));
  connect(obj,  SIGNAL(destroyed(QObject *)),
          this, SLOT(onWidgetDestroyed(QObject *)), Qt::DirectConnection);

  m_splitWidgets << obj;

  if(m_splitPlane != nullptr)
  {
    splitWidget3D->disableWidget();
  }
  else
  {
    auto segmentation = getSelectedSegmentations().first();
    splitWidget3D->setSegmentationBounds(segmentation->bounds());
  }
}

//------------------------------------------------------------------------
void SplitTool::onWidgetDestroyed(QObject *widget)
{
  if(widget)
  {
    Q_ASSERT(m_splitWidgets.contains(widget));
    m_splitWidgets.removeOne(widget);
  }
}

//------------------------------------------------------------------------
void SplitTool::onSplittingPlaneDefined(PlanarSplitWidgetPtr widget)
{
  auto valid = widget->planeIsValid();

  m_apply->setEnabled(valid);
  m_apply->setToolTip(tr("Cut segmentation using the defined cutting plane."));

  if(valid)
  {
    for(auto obj: m_splitWidgets)
    {
      auto splitWidget = dynamic_cast<PlanarSplitWidgetPtr>(obj);
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
  auto filter = dynamic_cast<FilterPtr>(sender());
  Q_ASSERT(m_executingTasks.keys().contains(filter));

  if(!filter->isAborted())
  {
    if (filter->numberOfOutputs() == 2)
    {
      auto sample   = QueryAdapter::samples(m_executingTasks.value(filter).segmentation);
      auto category = m_executingTasks.value(filter).segmentation->category();
      auto model    = getModel();
      auto number   = firstUnusedSegmentationNumber(model);

      SegmentationAdapterList  segmentations;
      SegmentationAdapterSList segmentationsList;

      for(auto i: {0, 1})
      {
        auto segmentation  = getFactory()->createSegmentation(m_executingTasks[filter].filter, i);
        segmentation->setCategory(category);
        segmentation->setNumber(number++);

        segmentationsList << segmentation;
        segmentations << segmentation.get();
      }

      auto undoStack        = getUndoStack();
      auto segmentation     = m_executingTasks[filter].segmentation.get();
      auto newSegmentation1 = segmentationsList.first()->data().toString();
      auto newSegmentation2 = segmentationsList.last()->data().toString();

      {
        WaitingCursor cursor;

        undoStack->beginMacro(tr("Split segmentation '%1' into '%2' and '%3'.").arg(segmentation->data().toString()).arg(newSegmentation1).arg(newSegmentation2));
        undoStack->push(new RemoveSegmentations(segmentation, model));
        undoStack->push(new AddSegmentations(segmentationsList, sample, model));
        undoStack->endMacro();
      }

      deactivateEventHandler();

      m_executingTasks[filter].segmentation->setBeingModified(false);

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
  return n == 1;
}

//------------------------------------------------------------------------
bool SplitTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return EditTool::acceptsVolumetricSegmentations(segmentations) && segmentations.size() == 1;
}

//------------------------------------------------------------------------
void SplitTool::abortTasks()
{
  for(auto data: m_executingTasks)
  {
    disconnect(data.filter.get(), SIGNAL(finished()),
               this,               SLOT(createSegmentations()));

    data.filter->abort();

    data.segmentation->setBeingModified(false);

    if(data.filter->thread() && !data.filter->thread()->wait(500))
    {
      data.filter->thread()->terminate();
    }
  }

  m_executingTasks.clear();
}
