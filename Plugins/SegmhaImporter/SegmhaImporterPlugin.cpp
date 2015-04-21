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
#include <Filters/SourceFilter.h>

using namespace ESPINA;

static const QString SEGMHA = "segmha";

static const Filter::Type SEGMHA_FILTER_V4 = "Segmha Importer";
static const Filter::Type SEGMHA_FILTER = "SegmhaReader";

//-----------------------------------------------------------------------------
FilterSPtr SegmhaImporterPlugin::SegmhaFilterFactory::createFilter(InputSList inputs,
                                                                   const Filter::Type &type,
                                                                   SchedulerSPtr scheduler) const
throw (Unknown_Filter_Exception)
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
    throw Unknown_Filter_Exception();
  }

  filter->setDataFactory(std::make_shared<MarchingCubesFromFetchedVolumetricData>());

  return filter;
}

//-----------------------------------------------------------------------------
FilterTypeList SegmhaImporterPlugin::SegmhaFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SEGMHA_FILTER;
  filters << SEGMHA_FILTER_V4;

  return filters;
}


//-----------------------------------------------------------------------------
SegmhaImporterPlugin::SegmhaImporterPlugin()
: m_undoStack    {nullptr}
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
  m_model       = context.model();
  m_factory     = context.factory();
  m_scheduler   = context.scheduler();
  m_undoStack   = context.undoStack();
}

//------------------------------------------------------------------------
AnalysisReaderSList SegmhaImporterPlugin::analysisReaders() const
{
  AnalysisReaderSList readers;

  readers << std::make_shared<SegmhaReader>();

  return readers;
}

//------------------------------------------------------------------------
NamedColorEngineSList SegmhaImporterPlugin::colorEngines() const
{
  return NamedColorEngineSList();
}

//------------------------------------------------------------------------
RepresentationFactorySList SegmhaImporterPlugin::representationFactories() const
{
  return RepresentationFactorySList();
}

//------------------------------------------------------------------------
QList<CategorizedTool> SegmhaImporterPlugin::tools() const
{
  return QList<CategorizedTool>();
}

//------------------------------------------------------------------------
QList<DockWidget *> SegmhaImporterPlugin::dockWidgets() const
{
  return QList<DockWidget *>();
}

//------------------------------------------------------------------------
ChannelExtensionFactorySList SegmhaImporterPlugin::channelExtensionFactories() const
{
  return ChannelExtensionFactorySList();
}

//------------------------------------------------------------------------
SegmentationExtensionFactorySList SegmhaImporterPlugin::segmentationExtensionFactories() const
{
  return SegmentationExtensionFactorySList();
}

//------------------------------------------------------------------------
FilterFactorySList SegmhaImporterPlugin::filterFactories() const
{
  FilterFactorySList factories;

  factories << std::make_shared<SegmhaFilterFactory>();

  return factories;
}

//------------------------------------------------------------------------
SettingsPanelSList SegmhaImporterPlugin::settingsPanels() const
{
  return SettingsPanelSList();
}

//------------------------------------------------------------------------
QList<MenuEntry> SegmhaImporterPlugin::menuEntries() const
{
  return QList<MenuEntry>();
}

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, ESPINA::SegmhaImporterPlugin)
