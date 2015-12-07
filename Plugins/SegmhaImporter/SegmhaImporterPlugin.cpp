/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// Plugin
#include "SegmhaImporterPlugin.h"

// ESPINA
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/IO/ReadOnlyFilter.h>
#include <Core/Utils/EspinaException.h>
#include <Filters/SourceFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

const Filter::Type SegmhaFilterFactory::SEGMHA_FILTER_V4 = "Segmha Importer";
const Filter::Type SegmhaFilterFactory::SEGMHA_FILTER    = "SegmhaReader";

//-----------------------------------------------------------------------------
FilterSPtr SegmhaFilterFactory::createFilter(InputSList          inputs,
                                             const Filter::Type &type,
                                             SchedulerSPtr       scheduler) const
{
  FilterSPtr filter;

  if(type == SEGMHA_FILTER)
  {
    filter = std::make_shared<SourceFilter>(inputs, type, scheduler);
  }
  else if(type == SEGMHA_FILTER_V4)
  {
    filter = std::make_shared<IO::SegFile::ReadOnlyFilter>(inputs, type);
  }
  else
  {
    auto what    = QObject::tr("Unable to create filter: %1").arg(type);
    auto details = QObject::tr("SeedGrowSegmentationFilterFactory::createFilter() -> Unknown filter: %1").arg(type);
    throw EspinaException(what, details);
  }

  if(!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }
  filter->setDataFactory(m_dataFactory);

  return filter;
}

//-----------------------------------------------------------------------------
FilterTypeList SegmhaFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SEGMHA_FILTER;
  filters << SEGMHA_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
SegmhaImporterPlugin::SegmhaImporterPlugin()
{
}


//-----------------------------------------------------------------------------
SegmhaImporterPlugin::~SegmhaImporterPlugin()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying SegmhaImporter Plugin";
//   qDebug() << "********************************************************";
}

//-----------------------------------------------------------------------------
void SegmhaImporterPlugin::init(Support::Context &context)
{
}

//------------------------------------------------------------------------
AnalysisReaderSList SegmhaImporterPlugin::analysisReaders() const
{
  AnalysisReaderSList readers;

  readers << std::make_shared<SegmhaReader>();

  return readers;
}

//------------------------------------------------------------------------
FilterFactorySList SegmhaImporterPlugin::filterFactories() const
{
  FilterFactorySList factories;

  factories << std::make_shared<SegmhaFilterFactory>();

  return factories;
}

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, ESPINA::SegmhaImporterPlugin)
