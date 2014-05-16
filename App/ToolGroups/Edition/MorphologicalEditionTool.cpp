/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "MorphologicalEditionTool.h"
#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/MorphologicalEditionFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/CloseFilter.h>
#include <Filters/OpenFilter.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>

#include <QAction>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>

const QString DILATE_RADIUS("MorphologicalEditionTools::DilateRadius");
const QString ERODE_RADIUS ("MorphologicalEditionTools::ErodeRadius");
const QString OPEN_RADIUS  ("MorphologicalEditionTools::OpenRadius");
const QString CLOSE_RADIUS ("MorphologicalEditionTools::CloseRadius");

using namespace EspINA;
using namespace EspINA::GUI;

const Filter::Type CLOSE_FILTER     = "CloseSegmentation";
const Filter::Type CLOSE_FILTER_V4  = "EditorToolBar::ClosingFilter";
const Filter::Type OPEN_FILTER      = "OpenSegmentation";
const Filter::Type OPEN_FILTER_V4   = "EditorToolBar::OpeningFilter";
const Filter::Type DILATE_FILTER    = "DilateSegmentation";
const Filter::Type DILATE_FILTER_V4 = "EditorToolBar::DilateFilter";
const Filter::Type ERODE_FILTER     = "ErodeSegmentation";
const Filter::Type ERODE_FILTER_V4  = "EditorToolBar::ErodeFilter";

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
    morphologicalFilter = FilterSPtr{new CloseFilter(inputs, CLOSE_FILTER, scheduler)};
  }
  else if (filter == OPEN_FILTER || filter == OPEN_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new OpenFilter(inputs, OPEN_FILTER, scheduler)};
  }
  else if (filter == DILATE_FILTER || filter == DILATE_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new DilateFilter(inputs, DILATE_FILTER, scheduler)};
  }
  else if (filter == ERODE_FILTER || filter == ERODE_FILTER_V4)
  {
    morphologicalFilter = FilterSPtr{new ErodeFilter(inputs, ERODE_FILTER, scheduler)};
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
: m_model(model)
, m_factory(factory)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
, m_filterFactory{new MorphologicalFilterFactory()}
, m_enabled(false)
, m_close (":/espina/close.png" , tr("Close selected segmentations" ))
, m_open  (":/espina/open.png"  , tr("Open selected segmentations"  ))
, m_dilate(":/espina/dilate.png", tr("Dilate selected segmentations"))
, m_erode (":/espina/erode.png" , tr("Erode selected segmentations" ))
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

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                             SLOT(updateAvailableActionsForSelection(SegmentationAdapterList)));


  m_fill = new QAction(QIcon(":/espina/fillHoles.svg"), tr("Fill internal holes in selected segmentations"), nullptr);
  connect(m_fill, SIGNAL(triggered(bool)),
          this, SLOT(fillHoles()));


  QSettings settings(CESVIMA, ESPINA);
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

  m_addition->setEnabled(value);
  m_subtract->setEnabled(value);
  m_fill    ->setEnabled(value);
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
  m_viewManager->setEventHandler(nullptr);

  auto inputs = m_viewManager->selection()->segmentations();

  if (inputs.size() > 1)
  {
    SegmentationAdapterList createdSegmentations;
    m_viewManager->selection()->clear();

    m_undoStack->beginMacro(tr("Merge Segmentations"));
    //      m_undoStack->push(new ImageLogicCommand(input,
    //                                              ImageLogicFilter::ADDITION,
    //                                              m_viewManager->activeTaxonomy(),
    //                                              m_model,
    //                                              createdSegmentations));
    //      m_model->emitSegmentationAdded(createdSegmentations);
    m_undoStack->endMacro();

  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::subtractSegmentations()
{
  //    m_viewManager->unsetActiveTool();
  //
  //    SegmentationList input = m_viewManager->selectedSegmentations();
  //    if (input.size() > 1)
  //    {
  //      SegmentationSList createdSegmentations;
  //      m_viewManager->clearSelection(true);
  //      m_undoStack->beginMacro("Subtract Segmentations");
  //      m_undoStack->push(new ImageLogicCommand(input,
  //                                              ImageLogicFilter::SUBTRACTION,
  //                                              m_viewManager->activeTaxonomy(),
  //                                              m_model,
  //                                              createdSegmentations));
  //      m_model->emitSegmentationAdded(createdSegmentations);
  //      m_undoStack->endMacro();
  //    }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::erodeSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() > 0)
  {
    int r = m_erode.radius();

    for (auto segmentation :  selection)
    {
      InputSList inputs;

      inputs << segmentation->asInput();

      auto adapter = m_factory->createFilter<ErodeFilter>(inputs, "Erode");
      auto filter  = adapter->get();

      filter->setRadius(r);
      filter->setDescription(tr("Erode %1").arg(segmentation->data(Qt::DisplayRole).toString()));

      TaskContext context;

      context.Task         = filter;
      context.Operation    = tr("Erode Segmentation");
      context.Segmentation = segmentation;

      m_executingMorpholocialTasks[filter.get()] = context;

//       connect(adapter.get(), SIGNAL(progress(int)),
//               this,          SLOT(onMorphologicalFilterFinished()));
      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onMorphologicalFilterFinished()));

      adapter->submit();
    }
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::dilateSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() > 0)
  {
    int r = m_dilate.radius();

    for (auto segmentation :  selection)
    {
      InputSList inputs;

      inputs << segmentation->asInput();

      auto adapter = m_factory->createFilter<DilateFilter>(inputs, "Dilate");
      auto filter  = adapter->get();

      filter->setRadius(r);
      filter->setDescription(tr("Dilate %1").arg(segmentation->data(Qt::DisplayRole).toString()));

      TaskContext context;

      context.Task         = filter;
      context.Operation    = tr("Dialte Segmentation");
      context.Segmentation = segmentation;

      m_executingMorpholocialTasks[filter.get()] = context;

//       connect(adapter.get(), SIGNAL(progress(int)),
//               this,          SLOT(onMorphologicalFilterFinished()));
      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onMorphologicalFilterFinished()));

      adapter->submit();
    }
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::openSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() > 0)
  {
    int r = m_open.radius();

    for (auto segmentation :  selection)
    {
      InputSList inputs;

      inputs << segmentation->asInput();

      auto adapter = m_factory->createFilter<OpenFilter>(inputs, "Open");
      auto filter  = adapter->get();

      filter->setRadius(r);
      filter->setDescription(tr("Open %1").arg(segmentation->data(Qt::DisplayRole).toString()));

      TaskContext context;

      context.Task         = filter;
      context.Operation    = tr("Open Segmentation");
      context.Segmentation = segmentation;

      m_executingMorpholocialTasks[filter.get()] = context;

//       connect(adapter.get(), SIGNAL(progress(int)),
//               this,          SLOT(onMorphologicalFilterFinished()));
      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onMorphologicalFilterFinished()));

      adapter->submit();
    }
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::closeSegmentations()
{
  m_viewManager->unsetActiveEventHandler();

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() > 0)
  {
    int r = m_close.radius();

    for (auto segmentation :  selection)
    {
      InputSList inputs;

      inputs << segmentation->asInput();

      auto adapter = m_factory->createFilter<CloseFilter>(inputs, "Close");
      auto filter  = adapter->get();

      filter->setRadius(r);
      filter->setDescription(tr("Close %1").arg(segmentation->data(Qt::DisplayRole).toString()));

      TaskContext context;

      context.Task         = filter;
      context.Operation    = tr("Close Segmentation");
      context.Segmentation = segmentation;

      m_executingMorpholocialTasks[filter.get()] = context;

//       connect(adapter.get(), SIGNAL(progress(int)),
//               this,          SLOT(onMorphologicalFilterFinished()));
      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(onMorphologicalFilterFinished()));

      adapter->submit();
    }
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::fillHoles()
{
  //    m_viewManager->unsetActiveTool();
  //
  //    SegmentationList input = m_viewManager->selectedSegmentations();
  //    if (input.size() > 0)
  //    {
  //      m_undoStack->push(new FillHolesCommand(input, m_model, m_viewManager));
  //    }
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
    m_close .toggleToolWidgets(false);
    m_open  .toggleToolWidgets(false);
    m_dilate.toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void MorphologicalEditionTool::updateAvailableActionsForSelection(SegmentationAdapterList selection)
{
  bool atLeastOneSegmentation = selection.size() > 0;

  m_close .setEnabled(atLeastOneSegmentation);
  m_open  .setEnabled(atLeastOneSegmentation);
  m_dilate.setEnabled(atLeastOneSegmentation);
  m_erode .setEnabled(atLeastOneSegmentation);
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
      auto title   = tr("Erode Segmentations");
      auto message = tr("%1 segmentation will be deleted by the ERODE operation.\n"
                        "Do you want to continue with the operation?").arg(name);

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


