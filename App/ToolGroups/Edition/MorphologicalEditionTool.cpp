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
#include "MorphologicalEditionTool.h"
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/MorphologicalEditionFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/CloseFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>

// Qt
#include <QAction>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>

const QString DILATE_RADIUS("MorphologicalEditionTools::DilateRadius");
const QString ERODE_RADIUS ("MorphologicalEditionTools::ErodeRadius");
const QString OPEN_RADIUS  ("MorphologicalEditionTools::OpenRadius");
const QString CLOSE_RADIUS ("MorphologicalEditionTools::CloseRadius");

using namespace ESPINA;
using namespace ESPINA::GUI;

const Filter::Type CLOSE_FILTER          = "CloseSegmentation";
const Filter::Type CLOSE_FILTER_V4       = "EditorToolBar::ClosingFilter";
const Filter::Type OPEN_FILTER           = "OpenSegmentation";
const Filter::Type OPEN_FILTER_V4        = "EditorToolBar::OpeningFilter";
const Filter::Type DILATE_FILTER         = "DilateSegmentation";
const Filter::Type DILATE_FILTER_V4      = "EditorToolBar::DilateFilter";
const Filter::Type ERODE_FILTER          = "ErodeSegmentation";
const Filter::Type ERODE_FILTER_V4       = "EditorToolBar::ErodeFilter";
const Filter::Type FILL_HOLES_FILTER     = "FillSegmentationHoles";
const Filter::Type FILL_HOLES_FILTER_V4  = "EditorToolBar::FillHolesFilter";
const Filter::Type IMAGE_LOGIC_FILTER    = "ImageLogicFilter";

//------------------------------------------------------------------------
FilterTypeList MorphologicalEditionTool::MorphologicalFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << CLOSE_FILTER;
  filters << CLOSE_FILTER_V4;
  filters << OPEN_FILTER;
  filters << OPEN_FILTER_V4;
  filters << DILATE_FILTER;
  filters << DILATE_FILTER_V4;
  filters << ERODE_FILTER;
  filters << ERODE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterSPtr MorphologicalEditionTool::MorphologicalFilterFactory::createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
throw (Unknown_Filter_Exception)
{
  FilterSPtr morphologicalFilter;

  if (!m_fetchBehaviour)
  {
    m_fetchBehaviour = FetchBehaviourSPtr{new MarchingCubesFromFetchedVolumetricData()};
  }

  if (filter == CLOSE_FILTER || filter == CLOSE_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new CloseFilter{inputs, CLOSE_FILTER, scheduler}};
  }
  else if (filter == OPEN_FILTER || filter == OPEN_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new OpenFilter{inputs, OPEN_FILTER, scheduler}};
  }
  else if (filter == DILATE_FILTER || filter == DILATE_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new DilateFilter{inputs, DILATE_FILTER, scheduler}};
  }
  else if (filter == ERODE_FILTER || filter == ERODE_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new ErodeFilter{inputs, ERODE_FILTER, scheduler}};
  }
  else if (filter == FILL_HOLES_FILTER || filter == FILL_HOLES_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new FillHolesFilter{inputs, FILL_HOLES_FILTER, scheduler}};
  }
  else if(filter == IMAGE_LOGIC_FILTER)
  {
    morphologicalFilter = FilterSPtr{new ImageLogicFilter{inputs, IMAGE_LOGIC_FILTER, scheduler}};
  }
  else
  {
    throw Unknown_Filter_Exception();
  }

  morphologicalFilter->setFetchBehaviour(m_fetchBehaviour);

  return morphologicalFilter;
}

//------------------------------------------------------------------------
MorphologicalEditionTool::MorphologicalEditionTool(ModelAdapterSPtr model,
                                                   ModelFactorySPtr factory,
                                                   ViewManagerSPtr  viewManager,
                                                   QUndoStack      *undoStack)
: m_model        {model}
, m_factory      {factory}
, m_viewManager  {viewManager}
, m_undoStack    {undoStack}
, m_filterFactory{new MorphologicalFilterFactory()}
, m_close        {":/espina/close.png" , tr("Close selected segmentations" )}
, m_open         {":/espina/open.png"  , tr("Open selected segmentations"  )}
, m_dilate       {":/espina/dilate.png", tr("Dilate selected segmentations")}
, m_erode        {":/espina/erode.png" , tr("Erode selected segmentations" )}
, m_enabled(false)
{
  m_factory->registerFilterFactory(m_filterFactory);

  m_addition = new QAction(QIcon(":/espina/add.svg"), tr("Merge selected segmentations"), nullptr);
  connect(m_addition, SIGNAL(triggered(bool)),
          this, SLOT(mergeSegmentations()));

  m_subtract = new QAction(QIcon(":/espina/remove.svg"), tr("Subtract selected segmentations"), nullptr);
  connect(m_subtract, SIGNAL(triggered(bool)),
          this, SLOT(subtractSegmentations()));

  connect(&m_close, SIGNAL(applyClicked()),
          this,     SLOT(closeSegmentations()));
  connect(&m_close, SIGNAL(toggled(bool)),
          this,     SLOT(onCloseToggled(bool)));

  connect(&m_open, SIGNAL(applyClicked()),
          this,    SLOT(openSegmentations()));

  connect(&m_open, SIGNAL(toggled(bool)),
          this,    SLOT(onOpenToggled(bool)));

  connect(&m_dilate, SIGNAL(applyClicked()),
          this,      SLOT(dilateSegmentations()));
  connect(&m_dilate, SIGNAL(toggled(bool)),
          this,      SLOT(onDilateToggled(bool)));

  connect(&m_erode, SIGNAL(applyClicked()),
          this,     SLOT(erodeSegmentations()));

  connect(&m_erode, SIGNAL(toggled(bool)),
          this,     SLOT(onErodeToggled(bool)));

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(updateAvailableActionsForSelection()));

  m_fill = new QAction(QIcon(":/espina/fillHoles.svg"), tr("Fill internal holes in selected segmentations"), nullptr);
  connect(m_fill, SIGNAL(triggered(bool)),
          this,   SLOT(fillHoles()));


  ESPINA_SETTINGS(settings);
  m_erode .setRadius(settings.value(ERODE_RADIUS,  3).toInt());
  m_dilate.setRadius(settings.value(DILATE_RADIUS, 3).toInt());
  m_open  .setRadius(settings.value(OPEN_RADIUS,   3).toInt());
  m_close .setRadius(settings.value(CLOSE_RADIUS,  3).toInt());
}

//------------------------------------------------------------------------
MorphologicalEditionTool::~MorphologicalEditionTool()
{
  delete m_addition;
  delete m_subtract;
  delete m_fill;
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::setEnabled(bool value)
{
  m_enabled = value;
  updateAvailableActionsForSelection();
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::enabled() const
{
  return m_enabled;
}

//------------------------------------------------------------------------
QList<QAction *> MorphologicalEditionTool::actions() const
{
  QList<QAction *> actions;

  actions << m_close .actions();
  actions << m_open  .actions();
  actions << m_dilate.actions();
  actions << m_erode .actions();
  actions << m_addition;
  actions << m_subtract;
  actions << m_fill;

  return actions;
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::mergeSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto segmentations = m_viewManager->selection()->segmentations();

  if (segmentations.size() > 1)
  {
    m_viewManager->selection()->clear();

    InputSList inputs;
    for(auto segmentation: segmentations)
      inputs << segmentation->asInput();

    auto adapter = m_factory->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
    auto filter = adapter->get();
    filter->setOperation(ImageLogicFilter::Operation::ADDITION);
    filter->setDescription("Segmentations Addition");

    ImageLogicContext context;

    context.Operation = ImageLogicFilter::Operation::ADDITION;
    context.Segmentations = segmentations;
    context.Task = adapter;

    m_executingImageLogicTasks.insert(filter.get(), context);

    connect(filter.get(), SIGNAL(finished()), this, SLOT(onImageLogicFilterFinished()));
    adapter->submit();
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::subtractSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto segmentations = m_viewManager->selection()->segmentations();

  if (segmentations.size() > 1)
  {
    m_viewManager->selection()->clear();

    InputSList inputs;
    for(auto segmentation: segmentations)
      inputs << segmentation->asInput();

    auto adapter = m_factory->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
    auto filter = adapter->get();
    filter->setOperation(ImageLogicFilter::Operation::SUBTRACTION);
    filter->setDescription("Segmentations Subtraction");

    ImageLogicContext context;

    context.Operation = ImageLogicFilter::Operation::SUBTRACTION;
    context.Segmentations = segmentations;
    context.Task = adapter;

    m_executingImageLogicTasks.insert(filter.get(), context);

    connect(filter.get(), SIGNAL(finished()), this, SLOT(onImageLogicFilterFinished()));
    adapter->submit();
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::closeSegmentations()
{
  launchCODE<CloseFilter>(CLOSE_FILTER, "Close", m_close.radius());
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::openSegmentations()
{
  launchCODE<OpenFilter>(OPEN_FILTER, "Open", m_open.radius());
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::dilateSegmentations()
{
  launchCODE<DilateFilter>(DILATE_FILTER, "Dilate", m_dilate.radius());
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::erodeSegmentations()
{
  launchCODE<DilateFilter>(ERODE_FILTER, "Erode", m_erode.radius());
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::fillHoles()
{
  m_viewManager->unsetActiveEventHandler();

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() > 0)
  {
    for (auto segmentation :  selection)
    {
      InputSList inputs;

      inputs << segmentation->asInput();

      auto adapter = m_factory->createFilter<FillHolesFilter>(inputs, FILL_HOLES_FILTER);
      auto filter  = adapter->get();

      filter->setDescription(tr("Fill %1 Holes").arg(segmentation->data(Qt::DisplayRole).toString()));

      FillHolesContext context;

      context.Task         = filter;
      context.Operation    = tr("Fill Segmentation Holes");
      context.Segmentation = segmentation;

      m_executingFillHolesTasks[filter.get()] = context;

      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onFillHolesFinished()));

      adapter->submit();
    }
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onCloseToggled(bool toggled)
{
  if (toggled)
  {
    m_viewManager->unsetActiveEventHandler();

    m_open  .toggleToolWidgets(false);
    m_dilate.toggleToolWidgets(false);
    m_erode .toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onOpenToggled(bool toggled)
{
  if (toggled)
  {
    m_viewManager->unsetActiveEventHandler();

    m_close .toggleToolWidgets(false);
    m_dilate.toggleToolWidgets(false);
    m_erode .toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onDilateToggled(bool toggled)
{
  if (toggled)
  {
    m_viewManager->unsetActiveEventHandler();

    m_close.toggleToolWidgets(false);
    m_open .toggleToolWidgets(false);
    m_erode.toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onErodeToggled(bool toggled)
{
  if (toggled)
  {
    m_viewManager->unsetActiveEventHandler();

    m_close .toggleToolWidgets(false);
    m_open  .toggleToolWidgets(false);
    m_dilate.toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::updateAvailableActionsForSelection()
{
  int listSize = m_viewManager->selection()->segmentations().size();

  bool atLeastOneSegmentation = listSize > 0;
  bool atLeasttwoSegmentations = listSize >= 2;

  m_addition->setEnabled(m_enabled && atLeasttwoSegmentations);
  m_subtract->setEnabled(m_enabled && atLeasttwoSegmentations);
  m_close .setEnabled(m_enabled && atLeastOneSegmentation);
  m_open  .setEnabled(m_enabled && atLeastOneSegmentation);
  m_dilate.setEnabled(m_enabled && atLeastOneSegmentation);
  m_erode .setEnabled(m_enabled && atLeastOneSegmentation);
  m_fill ->setEnabled(m_enabled && atLeastOneSegmentation);
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onMorphologicalFilterFinished()
{
  auto filter = dynamic_cast<MorphologicalEditionFilterPtr>(sender());

  if (!filter->isAborted())
  {
    auto context = m_executingMorpholocialTasks[filter];

    if (filter->isOutputEmpty())
    {
      auto name    = context.Segmentation->data(Qt::DisplayRole).toString();
      auto title   = context.Operation;
      auto message = tr("%1 segmentation will be deleted by %2 operation.\n"
                        "Do you want to continue with the operation?").arg(name).arg(context.Operation);

      if (DefaultDialogs::UserConfirmation(title, message))
      {
        m_undoStack->beginMacro(context.Operation);
        m_undoStack->push(new RemoveSegmentations(context.Segmentation, m_model));
        m_undoStack->endMacro();

        m_viewManager->updateViews();
      }
    }
    else
    {
      if (filter->numberOfOutputs() != 1) throw Filter::Undefined_Output_Exception();

      m_undoStack->beginMacro(context.Operation);
      m_undoStack->push(new ReplaceOutputCommand(context.Segmentation, getInput(context.Task, 0)));
      m_undoStack->endMacro();

      m_viewManager->updateSegmentationRepresentations(context.Segmentation);
      m_viewManager->updateViews();
    }
  }

  m_executingMorpholocialTasks.remove(filter);
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onFillHolesFinished()
{
  auto filter = dynamic_cast<FillHolesFilterPtr>(sender());

  if (!filter->isAborted())
  {
    auto context = m_executingFillHolesTasks[filter];

    if (filter->numberOfOutputs() != 1) throw Filter::Undefined_Output_Exception();

    m_undoStack->beginMacro(context.Operation);
    m_undoStack->push(new ReplaceOutputCommand(context.Segmentation, getInput(context.Task, 0)));
    m_undoStack->endMacro();

    m_viewManager->updateSegmentationRepresentations(context.Segmentation);
    m_viewManager->updateViews();
  }

  m_executingFillHolesTasks.remove(filter);
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::onImageLogicFilterFinished()
{
  auto filter = dynamic_cast<ImageLogicFilterPtr>(sender());

  if (!filter->isAborted())
  {
    Q_ASSERT(m_executingImageLogicTasks.keys().contains(filter));
    auto context = m_executingImageLogicTasks[filter];

    m_undoStack->beginMacro(filter->description());

    auto segmentation = m_factory->createSegmentation(context.Task, 0);
    segmentation->setCategory(context.Segmentations.first()->category());

    auto samples = QueryAdapter::samples(context.Segmentations.first());

    m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));

    for(auto segmentation: context.Segmentations)
      m_undoStack->push(new RemoveSegmentations(segmentation, m_model));

    m_undoStack->endMacro();

    m_viewManager->updateViews();
  }

  m_executingImageLogicTasks.remove(filter);
}
