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
#include "CODEHistory.h"
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
const Filter::Type ADDITION_FILTER       = "AdditionFilter";
const Filter::Type SUBSTRACTION_FILTER   = "SubstractionFilter";

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
FilterSPtr MorphologicalEditionTool::MorphologicalFilterFactory::createFilter(InputSList          inputs,
                                                                              const Filter::Type& filter,
                                                                              SchedulerSPtr       scheduler) const
throw (Unknown_Filter_Exception)
{
  FilterSPtr morphologicalFilter;

  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }

  if (isCloseFilter(filter))
  {
    morphologicalFilter = std::make_shared<CloseFilter>(inputs, CLOSE_FILTER, scheduler);
  }
  else if (isOpenFilter(filter))
  {
    morphologicalFilter = std::make_shared<OpenFilter>(inputs, OPEN_FILTER, scheduler);
  }
  else if (isDilateFilter(filter))
  {
    morphologicalFilter = std::make_shared<DilateFilter>(inputs, DILATE_FILTER, scheduler);
  }
  else if (isErodeFilter(filter))
  {
    morphologicalFilter = std::make_shared<ErodeFilter>(inputs, ERODE_FILTER, scheduler);
  }
  else if (isFillHolesFilter(filter))
  {
    morphologicalFilter = std::make_shared<FillHolesFilter>(inputs, FILL_HOLES_FILTER, scheduler);
  }
  else if (isAdditionFilter(filter) || isSubstractionFilter(filter))
  {
    morphologicalFilter = std::make_shared<ImageLogicFilter>(inputs, filter, scheduler);
  }
  else if(filter == IMAGE_LOGIC_FILTER) // Older versions didn't distinguish between addition/substraction
  {
    morphologicalFilter = std::make_shared<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER, scheduler);
  }
  else
  {
    throw Unknown_Filter_Exception();
  }

  morphologicalFilter->setDataFactory(m_dataFactory);

  return morphologicalFilter;
}

//------------------------------------------------------------------------
QList<Filter::Type> MorphologicalEditionTool::MorphologicalFilterFactory::availableFilterDelegates() const
{
  QList<Filter::Type> types;

  types << CLOSE_FILTER  << CLOSE_FILTER_V4
        << OPEN_FILTER   << OPEN_FILTER_V4
        << DILATE_FILTER << DILATE_FILTER_V4
        << ERODE_FILTER  << ERODE_FILTER_V4;

  return types;
}

//------------------------------------------------------------------------
FilterDelegateSPtr MorphologicalEditionTool::MorphologicalFilterFactory::createDelegate(FilterSPtr filter)
throw (Unknown_Filter_Type_Exception)
{
  QString title;

  auto type = filter->type();

  if (isCloseFilter(type))
  {
    title = tr("Close");
  }
  else if (isOpenFilter(type))
  {
    title = tr("Open");
  }
  else if (isDilateFilter(type))
  {
    title = tr("Dilate");
  }
  else if (isErodeFilter(type))
  {
    title = tr("Erode");
  }

  auto codeFilter = std::dynamic_pointer_cast<MorphologicalEditionFilter>(filter);

  return std::make_shared<CODEHistory>(title, codeFilter);

}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isCloseFilter(const Filter::Type &type) const
{
 return CLOSE_FILTER == type || CLOSE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isOpenFilter(const Filter::Type &type) const
{

  return OPEN_FILTER == type|| OPEN_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isDilateFilter(const Filter::Type &type) const
{
  return DILATE_FILTER == type || DILATE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isErodeFilter(const Filter::Type &type) const
{
  return ERODE_FILTER == type || ERODE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isFillHolesFilter(const Filter::Type &type) const
{
  return FILL_HOLES_FILTER == type || FILL_HOLES_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isAdditionFilter(const Filter::Type &type) const
{
  return ADDITION_FILTER == type;
}

//------------------------------------------------------------------------
bool MorphologicalEditionTool::MorphologicalFilterFactory::isSubstractionFilter(const Filter::Type &type) const
{
  return SUBSTRACTION_FILTER == type;
}



//------------------------------------------------------------------------
//------------------------------------------------------------------------
MorphologicalEditionTool::MorphologicalEditionTool(ModelAdapterSPtr          model,
                                                   ModelFactorySPtr          factory,
                                                   FilterDelegateFactorySPtr filterDelegateFactory,
                                                   ViewManagerSPtr           viewManager,
                                                   QUndoStack                *undoStack)
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
  filterDelegateFactory->registerFilterDelegateFactory(m_filterFactory);

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
    {
      inputs << segmentation->asInput();
    }

    auto filter = m_factory->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
    filter->setOperation(ImageLogicFilter::Operation::ADDITION);
    filter->setDescription("Segmentations Addition");

    ImageLogicContext context;

    context.Operation = ImageLogicFilter::Operation::ADDITION;
    context.Segmentations = segmentations;
    context.Task = filter;

    m_executingImageLogicTasks.insert(filter.get(), context);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(onImageLogicFilterFinished()));

    Task::submit(filter);
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

    auto filter = m_factory->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
    filter->setOperation(ImageLogicFilter::Operation::SUBTRACTION);
    filter->setDescription("Segmentations Subtraction");

    ImageLogicContext context;

    context.Operation = ImageLogicFilter::Operation::SUBTRACTION;
    context.Segmentations = segmentations;
    context.Task = filter;

    m_executingImageLogicTasks.insert(filter.get(), context);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(onImageLogicFilterFinished()));

    Task::submit(filter);
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
  launchCODE<ErodeFilter>(ERODE_FILTER, "Erode", m_erode.radius());
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

      auto filter = m_factory->createFilter<FillHolesFilter>(inputs, FILL_HOLES_FILTER);

      filter->setDescription(tr("Fill %1 Holes").arg(segmentation->data(Qt::DisplayRole).toString()));

      FillHolesContext context;

      context.Task         = filter;
      context.Operation    = tr("Fill Segmentation Holes");
      context.Segmentation = segmentation;

      m_executingFillHolesTasks[filter.get()] = context;

      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onFillHolesFinished()));

      Task::submit(filter);
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
  auto selection = m_viewManager->selection()->segmentations();

  bool hasRequiredData = true;

  for(auto seg: selection)
  {
    hasRequiredData &= hasVolumetricData(seg->output());
  }

  auto morphologicalEnabled = m_enabled && (selection.size() > 0) && hasRequiredData;
  auto logicalEnabled = m_enabled && (selection.size() >= 2) && hasRequiredData;

  m_addition->setEnabled(logicalEnabled);
  m_subtract->setEnabled(logicalEnabled);
  m_close .setEnabled(morphologicalEnabled);
  m_open  .setEnabled(morphologicalEnabled);
  m_dilate.setEnabled(morphologicalEnabled);
  m_erode .setEnabled(morphologicalEnabled);
  m_fill ->setEnabled(morphologicalEnabled);
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
