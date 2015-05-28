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


// //------------------------------------------------------------------------
// void RefineTool::onImageLogicFilterFinished()
// {
// }
