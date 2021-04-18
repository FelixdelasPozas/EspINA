/*

 Copyright (C) 2020 Felix de las Pozas Alvarez <felix.delaspozas@ctb.upm.es>

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

// Qt
#include <QString>

// Project
#include <Core/Types.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Filters/LibraryFiltersFactory.h>
#include <Filters/CleanSegmentationVoxelsFilter.h>
#include <Filters/CloseFilter.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/FillHoles2DFilter.h>
#include <Filters/ImageLogicFilter.h>
#include <Filters/OpenFilter.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <Filters/SliceInterpolationFilter.h>
#include <Filters/SplitFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core;

const Filter::Type CLOSE_FILTER               = "CloseSegmentation";
const Filter::Type CLOSE_FILTER_V4            = "EditorToolBar::ClosingFilter";
const Filter::Type OPEN_FILTER                = "OpenSegmentation";
const Filter::Type OPEN_FILTER_V4             = "EditorToolBar::OpeningFilter";
const Filter::Type DILATE_FILTER              = "DilateSegmentation";
const Filter::Type DILATE_FILTER_V4           = "EditorToolBar::DilateFilter";
const Filter::Type ERODE_FILTER               = "ErodeSegmentation";
const Filter::Type ERODE_FILTER_V4            = "EditorToolBar::ErodeFilter";
const Filter::Type FILL_HOLES_FILTER          = "FillSegmentationHoles";
const Filter::Type FILL_HOLES_FILTER_V4       = "EditorToolBar::FillHolesFilter";
const Filter::Type FILL_HOLES2D_FILTER        = "FillSegmentationHoles2D";
const Filter::Type IMAGE_LOGIC_FILTER         = "ImageLogicFilter";
const Filter::Type ADDITION_FILTER            = "AdditionFilter";
const Filter::Type SUBTRACTION_FILTER         = "SubstractionFilter";
const Filter::Type SLICE_INTERPOLATION_FILTER = "SliceInterpolationFilter";
const Filter::Type CLEAN_SEGMENTATION_FILTER  = "CleanSegmentationVoxelsFilter";
const Filter::Type SPLIT_FILTER               = "SplitFilter";
const Filter::Type SPLIT_FILTER_V4            = "EditorToolBar::SplitFilter";
const Filter::Type SOURCE_FILTER              = "FreeFormSource";
const Filter::Type SOURCE_FILTER_V4           = "::FreeFormSource";
const Filter::Type SGS_FILTER                 = "SeedGrowSegmentation";
const Filter::Type SGS_FILTER_V4              = "SeedGrowSegmentation::SeedGrowSegmentationFilter";
const Filter::Type SKELETON_FILTER            = "SkeletonSource";


//-----------------------------------------------------------------------------
ESPINA::FilterSPtr ESPINA::LibraryFiltersFactory::createFilter(InputSList inputs, const Filter::Type &filter, SchedulerSPtr scheduler) const
{
  FilterSPtr requestedFilter = nullptr;

  if (!m_mCubesData)
  {
    m_mCubesData = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }

  if(!m_rawData)
  {
    m_rawData = std::make_shared<RawDataFactory>();
  }

  if(filter.compare(SGS_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(SGS_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<SeedGrowSegmentationFilter>(inputs, SGS_FILTER, scheduler);
  }
  else if(filter.compare(CLOSE_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(CLOSE_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<CloseFilter>(inputs, CLOSE_FILTER, scheduler);
  }
  else if(filter.compare(OPEN_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(OPEN_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<OpenFilter>(inputs, OPEN_FILTER, scheduler);
  }
  else if(filter.compare(DILATE_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(DILATE_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<DilateFilter>(inputs, DILATE_FILTER, scheduler);
  }
  else if(filter.compare(ERODE_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(ERODE_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<ErodeFilter>(inputs, ERODE_FILTER, scheduler);
  }
  else if(filter.compare(FILL_HOLES_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(FILL_HOLES_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<FillHolesFilter>(inputs, FILL_HOLES_FILTER, scheduler);
  }
  else if(filter.compare(FILL_HOLES2D_FILTER, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<FillHoles2DFilter>(inputs, FILL_HOLES2D_FILTER, scheduler);
  }
  else if(filter.compare(IMAGE_LOGIC_FILTER, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<ImageLogicFilter>(inputs, IMAGE_LOGIC_FILTER, scheduler);
  }
  else if(filter.compare(ADDITION_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(SUBTRACTION_FILTER, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<ImageLogicFilter>(inputs, filter, scheduler);
  }
  else if(filter.compare(SLICE_INTERPOLATION_FILTER, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<SliceInterpolationFilter>(inputs, SLICE_INTERPOLATION_FILTER, scheduler);
  }
  else if(filter.compare(CLEAN_SEGMENTATION_FILTER, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<CleanSegmentationVoxelsFilter>(inputs, CLEAN_SEGMENTATION_FILTER, scheduler);
  }
  else if(filter.compare(SPLIT_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(SPLIT_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<SplitFilter>(inputs, SPLIT_FILTER, scheduler);
  }
  else if(filter.compare(SOURCE_FILTER, Qt::CaseInsensitive) == 0 || filter.compare(SOURCE_FILTER_V4, Qt::CaseInsensitive) == 0)
  {
    requestedFilter = std::make_shared<SourceFilter>(inputs, SOURCE_FILTER, scheduler);
  }

  if(requestedFilter)
  {
    requestedFilter->setDataFactory(m_mCubesData);
  }
  else
  {
    if(filter.compare(SKELETON_FILTER, Qt::CaseInsensitive) == 0)
    {
      requestedFilter = std::make_shared<SourceFilter>(inputs, SKELETON_FILTER, scheduler);
    }

    if(requestedFilter) requestedFilter->setDataFactory(m_rawData);
  }

  if(!requestedFilter)
  {
    const auto what    = QObject::tr("Unable to create filter %1").arg(filter);
    const auto details = QObject::tr("LibraryFiltersFactory::createFilter() -> Unknown filter type: %1").arg(filter);

    throw Core::Utils::EspinaException(what, details);
  }

  return requestedFilter;
}

//-----------------------------------------------------------------------------
const ESPINA::FilterTypeList ESPINA::LibraryFiltersFactory::providedFilters() const
{
  FilterTypeList types;

  types << CLOSE_FILTER;
  types << CLOSE_FILTER_V4;
  types << OPEN_FILTER;
  types << OPEN_FILTER_V4;
  types << DILATE_FILTER;
  types << DILATE_FILTER_V4;
  types << ERODE_FILTER;
  types << ERODE_FILTER_V4;
  types << FILL_HOLES_FILTER;
  types << FILL_HOLES_FILTER_V4;
  types << FILL_HOLES2D_FILTER;
  types << IMAGE_LOGIC_FILTER;
  types << ADDITION_FILTER;
  types << SUBTRACTION_FILTER;
  types << SLICE_INTERPOLATION_FILTER;
  types << CLEAN_SEGMENTATION_FILTER;
  types << SPLIT_FILTER;
  types << SPLIT_FILTER_V4;
  types << SOURCE_FILTER;
  types << SOURCE_FILTER_V4;
  types << SGS_FILTER;
  types << SGS_FILTER_V4;
  types << SKELETON_FILTER;

  return types;
}
