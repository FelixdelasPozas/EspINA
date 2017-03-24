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
#include "FillHoles2DTool.h"
#include "ImageLogicTool.h"

#include <App/ToolGroups/Edit/CODERefiner.h>
#include <Core/Analysis/Output.h>
#include <Core/Utils/EspinaException.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/Utils/SignalBlocker.h>
#include <Filters/CloseFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/FillHoles2DFilter.h>
#include <Filters/ImageLogicFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/OpenFilter.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <Support/Settings/Settings.h>
#include <Support/Widgets/EditTool.h>
#include <Undo/RemoveSegmentations.h>

// Qt
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;


const QString DILATE_RADIUS("EditTools::DilateRadius");
const QString ERODE_RADIUS ("EditTools::ErodeRadius");
const QString OPEN_RADIUS  ("EditTools::OpenRadius");
const QString CLOSE_RADIUS ("EditTools::CloseRadius");

// NOTE: there's a typo, don't change now or old files won't load correctly.
const Filter::Type EditionFilterFactory::CLOSE_FILTER         = "CloseSegmentation";
const Filter::Type EditionFilterFactory::CLOSE_FILTER_V4      = "EditorToolBar::ClosingFilter";
const Filter::Type EditionFilterFactory::OPEN_FILTER          = "OpenSegmentation";
const Filter::Type EditionFilterFactory::OPEN_FILTER_V4       = "EditorToolBar::OpeningFilter";
const Filter::Type EditionFilterFactory::DILATE_FILTER        = "DilateSegmentation";
const Filter::Type EditionFilterFactory::DILATE_FILTER_V4     = "EditorToolBar::DilateFilter";
const Filter::Type EditionFilterFactory::ERODE_FILTER         = "ErodeSegmentation";
const Filter::Type EditionFilterFactory::ERODE_FILTER_V4      = "EditorToolBar::ErodeFilter";
const Filter::Type EditionFilterFactory::FILL_HOLES_FILTER    = "FillSegmentationHoles";
const Filter::Type EditionFilterFactory::FILL_HOLES_FILTER_V4 = "EditorToolBar::FillHolesFilter";
const Filter::Type EditionFilterFactory::FILL_HOLES2D_FILTER  = "FillSegmentationHoles2D";
const Filter::Type EditionFilterFactory::IMAGE_LOGIC_FILTER   = "ImageLogicFilter";
const Filter::Type EditionFilterFactory::ADDITION_FILTER      = "AdditionFilter";
const Filter::Type EditionFilterFactory::SUBTRACTION_FILTER   = "SubstractionFilter";

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::CloseFilters()
{
  FilterTypeList filters;

  filters << CLOSE_FILTER;
  filters << CLOSE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::OpenFilters()
{
  FilterTypeList filters;

  filters << OPEN_FILTER;
  filters << OPEN_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::DilateFilters()
{

  FilterTypeList filters;

  filters << DILATE_FILTER;
  filters << DILATE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::ErodeFilters()
{
  FilterTypeList filters;

  filters << ERODE_FILTER;
  filters << ERODE_FILTER_V4;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::ImageLogicFilters()
{
  FilterTypeList filters;

  filters << IMAGE_LOGIC_FILTER;
  filters << ADDITION_FILTER;
  filters << SUBTRACTION_FILTER;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::FillHolesFilters()
{
  FilterTypeList filters;

  filters << FILL_HOLES_FILTER;
  filters << FILL_HOLES_FILTER_V4;
  filters << FILL_HOLES2D_FILTER;

  return filters;
}

//------------------------------------------------------------------------
FilterTypeList EditionFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << CloseFilters();
  filters << OpenFilters();
  filters << DilateFilters();
  filters << ErodeFilters();
  filters << FillHolesFilters();
  filters << ImageLogicFilters();

  return filters;
}

//------------------------------------------------------------------------
FilterSPtr EditionFilterFactory::createFilter(InputSList          inputs,
                                                    const Filter::Type& filter,
                                                    SchedulerSPtr       scheduler) const
{
  FilterSPtr editionFilter;

  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }

  if (isCloseFilter(filter))
  {
    editionFilter = std::make_shared<CloseFilter>(inputs, CLOSE_FILTER, scheduler);
  }
  else if (isOpenFilter(filter))
  {
    editionFilter = std::make_shared<OpenFilter>(inputs, OPEN_FILTER, scheduler);
  }
  else if (isDilateFilter(filter))
  {
    editionFilter = std::make_shared<DilateFilter>(inputs, DILATE_FILTER, scheduler);
  }
  else if (isErodeFilter(filter))
  {
    editionFilter = std::make_shared<ErodeFilter>(inputs, ERODE_FILTER, scheduler);
  }
  else if (isFillHolesFilter(filter))
  {
    editionFilter = std::make_shared<FillHolesFilter>(inputs, FILL_HOLES_FILTER, scheduler);
  }
  else if (isFillHoles2DFilter(filter))
  {
    editionFilter = std::make_shared<FillHoles2DFilter>(inputs, FILL_HOLES2D_FILTER, scheduler);
  }
  else if (isAdditionFilter(filter) || isSubstractionFilter(filter))
  {
    editionFilter = std::make_shared<ImageLogicFilter>(inputs, filter, scheduler);
  }
  else if(filter == IMAGE_LOGIC_FILTER) // Older versions didn't distinguish between addition/substraction
  {
    editionFilter = std::make_shared<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER, scheduler);
  }
  else
  {
    auto what    = QObject::tr("Unable to create filter %1").arg(filter);
    auto details = QObject::tr("EditionFilterFactory::createFilter() -> Unknown filter type: %1").arg(filter);
    throw EspinaException(what, details);
  }

  editionFilter->setDataFactory(m_dataFactory);

  return editionFilter;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isCloseFilter(const Filter::Type &type) const
{
 return CLOSE_FILTER == type || CLOSE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isOpenFilter(const Filter::Type &type) const
{

  return OPEN_FILTER == type|| OPEN_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isDilateFilter(const Filter::Type &type) const
{
  return DILATE_FILTER == type || DILATE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isErodeFilter(const Filter::Type &type) const
{
  return ERODE_FILTER == type || ERODE_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isFillHolesFilter(const Filter::Type &type) const
{
  return FILL_HOLES_FILTER == type || FILL_HOLES_FILTER_V4 == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isFillHoles2DFilter(const Filter::Type &type) const
{
  return FILL_HOLES2D_FILTER == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isAdditionFilter(const Filter::Type &type) const
{
  return ADDITION_FILTER == type;
}

//------------------------------------------------------------------------
bool EditionFilterFactory::isSubstractionFilter(const Filter::Type &type) const
{
  return SUBTRACTION_FILTER == type;
}


//-----------------------------------------------------------------------------
EditToolGroup::EditToolGroup(Support::FilterRefinerFactory &filgerRefiners,
                             Support::Context              &context,
                             QWidget                       *parent)
: ToolGroup{":/espina/toolgroup_refine.svg", tr("Edit"), parent}
, WithContext(context)
{
  auto editionFactory = std::make_shared<EditionFilterFactory>();
  context.factory()->registerFilterFactory(editionFactory);

  setToolTip(tr("Edit Existing Segmentations"));

  registerFilterRefiners(filgerRefiners);

  initManualEditionTool();
  initSplitTool();
  initCODETools();
  initFillHolesTools();
  initImageLogicTools();
}

//-----------------------------------------------------------------------------
void EditToolGroup::registerFilterRefiners(Support::FilterRefinerFactory &filterReginers)
{
  for (auto type : EditionFilterFactory::CloseFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Close")), type);
  }

  for (auto type : EditionFilterFactory::OpenFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Open")), type);
  }

  for (auto type : EditionFilterFactory::DilateFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Dilate")), type);
  }

  for (auto type : EditionFilterFactory::ErodeFilters())
  {
    filterReginers.registerFilterRefiner(std::make_shared<CODERefiner>(tr("Erode")), type);
  }
}

//-----------------------------------------------------------------------------
void EditToolGroup::initManualEditionTool()
{
  auto manualEdition = std::make_shared<ManualEditionTool>(getContext());
  manualEdition->setOrder("1");
  addTool(manualEdition);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initSplitTool()
{
  auto split = std::make_shared<SplitTool>(getContext());
  split->setOrder("3-3");
  addTool(split);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initCODETools()
{
  auto close  = std::make_shared<CODETool<CloseFilter>> (EditionFilterFactory::CLOSE_FILTER, "CloseTool",  tr("Close"), ":/espina/morphological_close.svg",  tr("Close Segmentations") , getContext());
  auto open   = std::make_shared<CODETool<OpenFilter>>  (EditionFilterFactory::OPEN_FILTER,  "OpenTool",   tr("Open"),  ":/espina/morphological_open.svg",   tr("Open Segmentations")  , getContext());
  auto dilate = std::make_shared<CODETool<DilateFilter>>(EditionFilterFactory::DILATE_FILTER,"DilateTool", tr("Dilate"),":/espina/morphological_dilate.svg", tr("Dilate Segmentations"), getContext());
  auto erode  = std::make_shared<CODETool<ErodeFilter>> (EditionFilterFactory::ERODE_FILTER, "ErodeTool",  tr("Erode"), ":/espina/morphological_erode.svg",  tr("Erode Segmentations") , getContext());

  close ->setOrder("2-0");
  open  ->setOrder("2-1");
  dilate->setOrder("2-2");
  erode ->setOrder("2-3");

  addTool(close);
  addTool(open);
  addTool(dilate);
  addTool(erode);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initFillHolesTools()
{
  auto fillHoles   = std::make_shared<FillHolesTool>(getContext());
  auto fillHoles2D = std::make_shared<FillHoles2DTool>(getContext());

  fillHoles  ->setOrder("2-4");
  fillHoles2D->setOrder("2-5");

  addTool(fillHoles);
  addTool(fillHoles2D);
}

//-----------------------------------------------------------------------------
void EditToolGroup::initImageLogicTools()
{
  auto addition  = std::make_shared<ImageLogicTool>("Merge", ":/espina/logical_union.svg", tr("Merge selected segmentations"), getContext());
  addition->setOperation(ImageLogicFilter::Operation::ADDITION);

  auto subtract = std::make_shared<ImageLogicTool>("Substract", ":/espina/logical_difference.svg", tr("Subtract selected segmentations"), getContext());
  subtract->setOperation(ImageLogicFilter::Operation::SUBTRACTION);
  subtract->removeOnSubtract(false);

  auto subtractAndErase = std::make_shared<ImageLogicTool>("SubstractErase", ":/espina/logical_difference_erase.svg", tr("Subtract selected segmentations deleting the subtracted segmentations"), getContext());
  subtractAndErase->setOperation(ImageLogicFilter::Operation::SUBTRACTION);

  addition        ->setOrder("3-0");
  subtract        ->setOrder("3-1");
  subtractAndErase->setOrder("3-2");

  addTool(addition);
  addTool(subtract);
  addTool(subtractAndErase);
}
