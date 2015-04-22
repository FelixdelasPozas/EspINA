/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ESPINA_SEGMHA_IMPORTER_H
#define ESPINA_SEGMHA_IMPORTER_H

#include "SegmhaImporterPlugin_Export.h"

// ESPINA
#include <Support/Plugin.h>
#include "SegmhaReader.h"

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class SegmhaImporterPlugin_EXPORT SegmhaImporterPlugin
  : public Plugin
  {
    Q_OBJECT
    Q_INTERFACES(ESPINA::Plugin)

    class SegmhaFilterFactory
    : public FilterFactory
    {
    public:
      virtual ~SegmhaFilterFactory()
      {};

      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);
    };

  public:
    explicit SegmhaImporterPlugin();
    virtual ~SegmhaImporterPlugin();

    virtual void init(Support::Context &context);

    virtual ChannelExtensionFactorySList channelExtensionFactories() const;

    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    virtual FilterFactorySList filterFactories() const;

    virtual AnalysisReaderSList analysisReaders() const;

    virtual NamedColorEngineSList colorEngines() const;

    virtual RepresentationFactorySList representationFactories() const;

    virtual QList<CategorizedTool> tools() const;

    virtual QList<DockWidget *> dockWidgets() const;

    virtual SettingsPanelSList settingsPanels() const;

    virtual QList<MenuEntry> menuEntries() const;

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    SchedulerSPtr    m_scheduler;
    QUndoStack*      m_undoStack;
  };
} // namespace ESPINA

#endif// SEGMHAIMPORTER_H
