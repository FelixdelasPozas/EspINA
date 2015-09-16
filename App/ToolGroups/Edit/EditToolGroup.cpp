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
#include "EditToolGroup.h"

#include "ManualEditionTool.h"
#include "SplitTool.h"
#include "CODETool.h"
#include "FillHolesTool.h"
#include "ImageLogicTool.h"

#include <App/ToolGroups/Edit/CODERefiner.h>
#include <Core/Analysis/Output.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/CloseFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/ImageLogicFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/OpenFilter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Widgets/EditTool.h>
#include <Undo/AddSegmentations.h>
#include <Undo/DrawUndoCommand.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/RemoveSegmentations.h>
#include <Undo/ReplaceOutputCommand.h>

// Qt
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;


const QString DILATE_RADIUS("EditTools::DilateRadius");
const QString ERODE_RADIUS ("EditTools::ErodeRadius");
const QString OPEN_RADIUS  ("EditTools::OpenRadius");
const QString CLOSE_RADIUS ("EditTools::CloseRadius");

//   ESPINA_SETTINGS(settings);
//   m_erode .setRadius(settings.value(ERODE_RADIUS,  3).toInt());
//   m_dilate.setRadius(settings.value(DILATE_RADIUS, 3).toInt());
//   m_open  .setRadius(settings.value(OPEN_RADIUS,   3).toInt());
//   m_close .setRadius(settings.value(CLOSE_RADIUS,  3).toInt());

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

class MorphologicalFilterFactory
: public FilterFactory
{
public:
  static FilterTypeList CloseFilters();

  static FilterTypeList OpenFilters();

  static FilterTypeList DilateFilters();

  static FilterTypeList ErodeFilters();

private:
  virtual FilterTypeList providedFilters() const;

  virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
  throw (Unknown_Filter_Exception);

private:
  bool isCloseFilter       (const Filter::Type &type) const;
  bool isOpenFilter        (const Filter::Type &type) const;
  bool isDilateFilter      (const Filter::Type &type) const;
  bool isErodeFilter       (const Filter::Type &type) const;
  bool isFillHolesFilter   (const Filter::Type &type) const;
  bool isAdditionFilter    (const Filter::Type &type) const;
  bool isSubstractionFilter(const Filter::Type &type) const;

private:
  mutable DataFactorySPtr m_dataFactory;
};

//------------------------------------------------------------------------
FilterTypeList MorphologicalFilterFactory::CloseFilters()
{
  FilterTypeList filters;

  filters << CLOSE_FILTER;
  filters << CLOSE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList MorphologicalFilterFactory::OpenFilters()
{
  FilterTypeList filters;

  filters << OPEN_FILTER;
  filters << OPEN_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList MorphologicalFilterFactory::DilateFilters()
{

  FilterTypeList filters;

  filters << DILATE_FILTER;
  filters << DILATE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList MorphologicalFilterFactory::ErodeFilters()
{
  FilterTypeList filters;

  filters << ERODE_FILTER;
  filters << ERODE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList MorphologicalFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << CloseFilters();
  filters << OpenFilters();
  filters << DilateFilters();
  filters << ErodeFilters();

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


//-----------------------------------------------------------------------------
EditToolGroup::EditToolGroup(Support::FilterRefinerRegister &filgerRefiners,
                                 Support::Context               &context)
: ToolGroup{":/espina/toolgroup_refine.svg", tr("Edit")}
, WithContext(context)
{
  auto morphologicalFactory = std::make_shared<MorphologicalFilterFactory>();
  context.factory()->registerFilterFactory(morphologicalFactory);

  setToolTip(tr("Edit Existing Segmentations"));

  registerFilterRefiners(filgerRefiners);

  initManualEditionTool();
  initSplitTool();
  initCODETools();
  initFillHolesTool();
  initImageLogicTools();
}

//-----------------------------------------------------------------------------
EditToolGroup::~EditToolGroup()
{
}

//-----------------------------------------------------------------------------
void EditToolGroup::registerFilterRefiners(Support::FilterRefinerRegister &filterReginers)
{
  for (auto type : MorphologicalFilterFactory::CloseFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Close")), type);
  }

  for (auto type : MorphologicalFilterFactory::OpenFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Open")), type);
  }

  for (auto type : MorphologicalFilterFactory::DilateFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Dilate")), type);
  }

  for (auto type : MorphologicalFilterFactory::ErodeFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Erode")), type);
  }
}

//-----------------------------------------------------------------------------
void EditToolGroup::initManualEditionTool()
{
  auto manualEdition = std::make_shared<ManualEditionTool>(getContext());

  manualEdition->setOrder("1");

  connect(manualEdition.get(), SIGNAL(voxelsDeleted(ViewItemAdapterPtr)),
          this,                SLOT(onVoxelDeletion(ViewItemAdapterPtr)));

  addTool(manualEdition);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initSplitTool()
{
  auto split = std::make_shared<SplitTool>(getContext());
  split->setOrder("3-2");
  addTool(split);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initCODETools()
{
  auto close  = std::make_shared<CODETool<CloseFilter>> (CLOSE_FILTER, "CloseTool",  tr("Close"), ":/espina/morphological_close.svg",  tr("Close Segmentations") , getContext());
  auto open   = std::make_shared<CODETool<OpenFilter>>  (OPEN_FILTER,  "OpenTool",   tr("Open"),  ":/espina/morphological_open.svg",   tr("Open Segmentations")  , getContext());
  auto dilate = std::make_shared<CODETool<DilateFilter>>(DILATE_FILTER,"DilateTool", tr("Dilate"),":/espina/morphological_dilate.svg", tr("Dilate Segmentations"), getContext());
  auto erode  = std::make_shared<CODETool<ErodeFilter>> (ERODE_FILTER, "ErodeTool",  tr("Erode"), ":/espina/morphological_erode.svg",  tr("Erode Segmentations") , getContext());

  close->setOrder("2-0");
  open->setOrder("2-1");
  dilate->setOrder("2-2");
  erode->setOrder("2-3");

  addTool(close);
  addTool(open);
  addTool(dilate);
  addTool(erode);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initFillHolesTool()
{
  auto fillHoles = std::make_shared<FillHolesTool>(getContext());

  fillHoles->setOrder("2-4");

  addTool(fillHoles);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initImageLogicTools()
{
  auto addition  = std::make_shared<ImageLogicTool>("Merge", ":/espina/logical_union.svg",    tr("Merge Selected Segmentations"),     getContext());
  addition->setOperation(ImageLogicFilter::Operation::ADDITION);

  auto substract = std::make_shared<ImageLogicTool>("Substract", ":/espina/logical_difference.svg", tr("Substract Selected Segmentations"), getContext());
  substract->setOperation(ImageLogicFilter::Operation::SUBTRACTION);

  addition->setOrder("3-0");
  substract->setOrder("3-1");

  addTool(addition);
  addTool(substract);
}

//-----------------------------------------------------------------------------
void EditToolGroup::onVoxelDeletion(ViewItemAdapterPtr item)
{
  Q_ASSERT(item && isSegmentation(item) && hasVolumetricData(item->output()));

  bool removeSegmentation = false;

  auto segmentation = segmentationPtr(item);

  {
    auto volume = writeLockVolume(segmentation->output());

    if (volume->isEmpty())
    {
      removeSegmentation = true;
    }
    else
    {
      fitToContents(volume, SEG_BG_VALUE);
    }
  }

  if (removeSegmentation)
  {
    getViewState().setEventHandler(EventHandlerSPtr());

    auto name  = segmentation->data(Qt::DisplayRole).toString();
    auto title = tr("Deleting segmentation");
    auto msg   = tr("%1 will be deleted because all its voxels were erased.").arg(name);

    DefaultDialogs::InformationMessage(msg, title);

    auto undoStack = getUndoStack();

    undoStack->blockSignals(true);
    undoStack->undo();
    undoStack->blockSignals(false);

    undoStack->beginMacro(tr("Remove Segmentation"));
    undoStack->push(new RemoveSegmentations(segmentation, getModel()));
    undoStack->endMacro();
  }
}
