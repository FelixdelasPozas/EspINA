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
#include "RefineTool.h"

#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/EspinaSettings.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/CloseFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/ImageLogicFilter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <App/ToolGroups/Refine/CODEHistory.h>

// Qt
#include <QAction>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>

const QString DILATE_RADIUS("RefineTools::DilateRadius");
const QString ERODE_RADIUS ("RefineTools::ErodeRadius");
const QString OPEN_RADIUS  ("RefineTools::OpenRadius");
const QString CLOSE_RADIUS ("RefineTools::CloseRadius");

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Support::Widgets;


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
FilterTypeList MorphologicalFilterFactory::providedFilters() const
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
FilterSPtr MorphologicalFilterFactory::createFilter(InputSList          inputs,
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
QList<Filter::Type> MorphologicalFilterFactory::availableFilterDelegates() const
{
  QList<Filter::Type> types;

  types << CLOSE_FILTER  << CLOSE_FILTER_V4
        << OPEN_FILTER   << OPEN_FILTER_V4
        << DILATE_FILTER << DILATE_FILTER_V4
        << ERODE_FILTER  << ERODE_FILTER_V4;

  return types;
}

//------------------------------------------------------------------------
FilterDelegateSPtr MorphologicalFilterFactory::createDelegate(SegmentationAdapterPtr segmentation,
                                                              FilterSPtr             filter)
throw (Unknown_Filter_Type_Exception)
{
  QString title;

  auto type = filter->type();

  if (isCloseFilter(type))
  {
    title = QObject::tr("Close");
  }
  else if (isOpenFilter(type))
  {
    title = QObject::tr("Open");
  }
  else if (isDilateFilter(type))
  {
    title = QObject::tr("Dilate");
  }
  else if (isErodeFilter(type))
  {
    title = QObject::tr("Erode");
  }

  auto codeFilter = std::dynamic_pointer_cast<MorphologicalEditionFilter>(filter);

  return std::make_shared<CODEHistory>(title, codeFilter);

}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isCloseFilter(const Filter::Type &type) const
{
 return CLOSE_FILTER == type || CLOSE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isOpenFilter(const Filter::Type &type) const
{

  return OPEN_FILTER == type|| OPEN_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isDilateFilter(const Filter::Type &type) const
{
  return DILATE_FILTER == type || DILATE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isErodeFilter(const Filter::Type &type) const
{
  return ERODE_FILTER == type || ERODE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isFillHolesFilter(const Filter::Type &type) const
{
  return FILL_HOLES_FILTER == type || FILL_HOLES_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isAdditionFilter(const Filter::Type &type) const
{
  return ADDITION_FILTER == type;
}

//------------------------------------------------------------------------
bool MorphologicalFilterFactory::isSubstractionFilter(const Filter::Type &type) const
{
  return SUBSTRACTION_FILTER == type;
}



//------------------------------------------------------------------------
//------------------------------------------------------------------------
RefineTool::RefineTool(const QString& icon, const QString& tooltip, Support::Context& context)
: ProgressTool(icon, tooltip, context)
{
  connect(getSelection(context).get(), SIGNAL(selectionChanged()),
          this,                        SLOT(updateStatus()));
/*
  m_addition = Tool::createAction(":/espina/add.svg", tr("Merge selected segmentations"), this);
  connect(m_addition, SIGNAL(triggered(bool)),
          this, SLOT(mergeSegmentations()));

  m_subtract = Tool::createAction(":/espina/remove.svg", tr("Subtract selected segmentations"), this);
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


  m_fill = Tool::createAction(":/espina/fillHoles.svg", tr("Fill internal holes in selected segmentations"), this);
  connect(m_fill, SIGNAL(triggered(bool)),
          this,   SLOT(fillHoles()));


  ESPINA_SETTINGS(settings);
  m_erode .setRadius(settings.value(ERODE_RADIUS,  3).toInt());
  m_dilate.setRadius(settings.value(DILATE_RADIUS, 3).toInt());
  m_open  .setRadius(settings.value(OPEN_RADIUS,   3).toInt());
  m_close .setRadius(settings.value(CLOSE_RADIUS,  3).toInt());

  updateAvailableActionsForSelection(); */
}

//------------------------------------------------------------------------
RefineTool::~RefineTool()
{
}

// //------------------------------------------------------------------------
// void RefineTool::mergeSegmentations()
// {
//   m_context.viewState().setEventHandler(nullptr);
//
//   auto selection = getSelection(m_context);
//   auto segmentations    = selection->segmentations();
//
//   if (segmentations.size() > 1)
//   {
//     selection->clear();
//
//     InputSList inputs;
//     for(auto segmentation: segmentations)
//     {
//       inputs << segmentation->asInput();
//     }
//
//     auto filter = m_context.factory()->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
//     filter->setOperation(ImageLogicFilter::Operation::ADDITION);
//     filter->setDescription("Segmentations Addition");
//
//     ImageLogicContext context;
//
//     context.Operation = ImageLogicFilter::Operation::ADDITION;
//     context.Segmentations = segmentations;
//     context.Task = filter;
//
//     m_executingImageLogicTasks.insert(filter.get(), context);
//
//     connect(filter.get(), SIGNAL(finished()),
//             this,         SLOT(onImageLogicFilterFinished()));
//
//     Task::submit(filter);
//   }
// }
//
// //------------------------------------------------------------------------
// void RefineTool::subtractSegmentations()
// {
//   m_context.viewState().setEventHandler(nullptr);
//
//   auto selection = getSelection(m_context);
//   auto segmentations    = selection->segmentations();
//
//   if (segmentations.size() > 1)
//   {
//     selection->clear();
//
//     InputSList inputs;
//     for(auto segmentation: segmentations)
//     {
//       inputs << segmentation->asInput();
//     }
//
//     auto filter = m_context.factory()->createFilter<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER);
//     filter->setOperation(ImageLogicFilter::Operation::SUBTRACTION);
//     filter->setDescription("Segmentations Subtraction");
//
//     ImageLogicContext context;
//
//     context.Operation = ImageLogicFilter::Operation::SUBTRACTION;
//     context.Segmentations = segmentations;
//     context.Task = filter;
//
//     m_executingImageLogicTasks.insert(filter.get(), context);
//
//     connect(filter.get(), SIGNAL(finished()),
//             this,         SLOT(onImageLogicFilterFinished()));
//
//     Task::submit(filter);
//   }
// }
//
// //------------------------------------------------------------------------
// void RefineTool::closeSegmentations()
// {
//   launchCODE<CloseFilter>(CLOSE_FILTER, "Close", m_close.radius());
// }
//
// //------------------------------------------------------------------------
// void RefineTool::openSegmentations()
// {
//   launchCODE<OpenFilter>(OPEN_FILTER, "Open", m_open.radius());
// }
//
// //------------------------------------------------------------------------
// void RefineTool::dilateSegmentations()
// {
//   launchCODE<DilateFilter>(DILATE_FILTER, "Dilate", m_dilate.radius());
// }
//
// //------------------------------------------------------------------------
// void RefineTool::erodeSegmentations()
// {
//   launchCODE<ErodeFilter>(ERODE_FILTER, "Erode", m_erode.radius());
// }

// //------------------------------------------------------------------------
// void RefineTool::fillHoles()
// {
//   m_context.viewState().setEventHandler(nullptr); // NOTE: Reconsider unsetActiveEventHandler
//
//   auto selection = getSelection(m_context)->segmentations();
//
//   if (selection.size() > 0)
//   {
//     for (auto segmentation :  selection)
//     {
//       InputSList inputs;
//
//       inputs << segmentation->asInput();
//
//       auto filter = m_context.factory()->createFilter<FillHolesFilter>(inputs, FILL_HOLES_FILTER);
//
//       filter->setDescription(tr("Fill %1 Holes").arg(segmentation->data(Qt::DisplayRole).toString()));
//
//       FillHolesContext context;
//
//       context.Task         = filter;
//       context.Operation    = tr("Fill Segmentation Holes");
//       context.Segmentation = segmentation;
//
//       m_executingFillHolesTasks[filter.get()] = context;
//
//       connect(filter.get(), SIGNAL(finished()),
//               this,         SLOT(onFillHolesFinished()));
//
//       Task::submit(filter);
//     }
//   }
// }


//------------------------------------------------------------------------
bool RefineTool::acceptsVolumetricSegmenations(SegmentationAdapterList segmentations)
{
  bool hasRequiredData = true;

  for(auto segmentation : segmentations)
  {
    hasRequiredData &= hasVolumetricData(segmentation->output()); // TODO: virtual puro
  }

  return hasRequiredData;
}

//------------------------------------------------------------------------
void RefineTool::updateStatus()
{
  auto selection = getSelection(context())->segmentations();


  setEnabled(acceptsNInputs(selection.size()) && acceptsSelection(selection));
}

//------------------------------------------------------------------------
void RefineTool::onToolEnabled(bool enabled)
{
  updateStatus();
}

//------------------------------------------------------------------------
bool RefineTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  return true;
}



//   auto morphologicalEnabled = isEnabled() && (selection.size() >  0) && hasRequiredData;
//   auto logicalEnabled       = isEnabled() && (selection.size() >= 2) && hasRequiredData;
//
//   m_addition->setEnabled(logicalEnabled);
//   m_subtract->setEnabled(logicalEnabled);
//   m_close .setEnabled2(morphologicalEnabled);
//   m_open  .setEnabled2(morphologicalEnabled);
//   m_dilate.setEnabled2(morphologicalEnabled);
//   m_erode .setEnabled2(morphologicalEnabled);
//   m_fill ->setEnabled(morphologicalEnabled);
// }
//
// //------------------------------------------------------------------------
// void RefineTool::onMorphologicalFilterFinished()
// {
// }
//
// //------------------------------------------------------------------------
// void RefineTool::onFillHolesFinished()
// {
//   auto filter = dynamic_cast<FillHolesFilterPtr>(sender());
//
//   if (!filter->isAborted())
//   {
//     auto taskContext = m_executingFillHolesTasks[filter];
//
//     if (filter->numberOfOutputs() != 1) throw Filter::Undefined_Output_Exception();
//
//     auto undoStack = m_context.undoStack();
//
//     undoStack->beginMacro(taskContext.Operation);
//     undoStack->push(new ReplaceOutputCommand(taskContext.Segmentation, getInput(taskContext.Task, 0)));
//     undoStack->endMacro();
//   }
//
//   m_executingFillHolesTasks.remove(filter);
// }
//
// //------------------------------------------------------------------------
// void RefineTool::onImageLogicFilterFinished()
// {
//   auto filter = dynamic_cast<ImageLogicFilterPtr>(sender());
//
//   if (!filter->isAborted())
//   {
//     Q_ASSERT(m_executingImageLogicTasks.keys().contains(filter));
//     auto context = m_executingImageLogicTasks[filter];
//
//     auto undoStack = m_context.undoStack();
//
//     undoStack->beginMacro(filter->description());
//
//     auto segmentation = m_context.factory()->createSegmentation(context.Task, 0);
//     segmentation->setCategory(context.Segmentations.first()->category());
//
//     auto samples = QueryAdapter::samples(context.Segmentations.first());
//
//     undoStack->push(new AddSegmentations(segmentation, samples, m_context.model()));
//
//     for(auto segmentation: context.Segmentations)
//       undoStack->push(new RemoveSegmentations(segmentation, m_context.model()));
//
//     undoStack->endMacro();
//
//     //REVIEW m_viewManager->updateViews();
//   }
//
//   m_executingImageLogicTasks.remove(filter);
// }
