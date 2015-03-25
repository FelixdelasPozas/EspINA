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

#ifndef APPOSITION_SURFACE_PLUGIN_H
#define APPOSITION_SURFACE_PLUGIN_H

#include "AppositionSurfacePlugin_Export.h"

// Plugin
#include "Core/Extensions/AppositionSurfaceExtension.h"

// ESPINA
#include <Support/ViewManager.h>
#include <Support/Plugin.h>
#include <Core/Analysis/Input.h>
#include <Core/Analysis/DataFactory.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/EspinaTypes.h>

namespace ESPINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfacePlugin
  : public Plugin
  {
    Q_OBJECT
    Q_INTERFACES(ESPINA::Plugin)

    class ASFilterFactory
    : public FilterFactory
    {
        virtual FilterTypeList providedFilters() const;
        virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);
    };

  public:
    explicit AppositionSurfacePlugin();
    virtual ~AppositionSurfacePlugin();

    virtual void init(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager,
                      ModelFactorySPtr factory,
                      SchedulerSPtr    scheduler,
                      QUndoStack      *undoStack);

    virtual ChannelExtensionFactorySList channelExtensionFactories() const;

    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    virtual NamedColorEngineSList colorEngines() const;

    virtual RepresentationFactorySList representationFactories() const;

    virtual QList<CategorizedTool> tools() const;

    virtual QList<DockWidget *> dockWidgets() const;

    virtual SettingsPanelSList settingsPanels() const;

    virtual QList<MenuEntry> menuEntries() const;

    virtual AnalysisReaderSList analysisReaders() const;

    virtual FilterFactorySList filterFactories() const;

  public slots:
    void createSASAnalysis();

    void segmentationsAdded(ViewItemAdapterSList segmentations);

    void finishedTask();

  private:
    static bool isSAS(ItemAdapterSPtr item);

  private:
    struct Data
    {
      FilterSPtr              adapter;
      SegmentationAdapterSPtr segmentation;

      Data(FilterSPtr adapterP, SegmentationAdapterSPtr segmentationP)
      : adapter{adapterP}, segmentation{segmentationP}
      {};

      Data(): adapter{nullptr}, segmentation{nullptr}
      {};
    };

    static bool isValidSynapse(SegmentationAdapterPtr segmentation);

  private:
    ModelAdapterSPtr                 m_model;
    ModelFactorySPtr                 m_factory;
    ViewManagerSPtr                  m_viewManager;
    SchedulerSPtr                    m_scheduler;
    QUndoStack                      *m_undoStack;
    SettingsPanelSPtr                m_settings;
    SegmentationExtensionFactorySPtr m_extensionFactory;
    MenuEntry                        m_menuEntry;
    FilterFactorySPtr                m_filterFactory;
    bool                             m_delayedAnalysis;
    SegmentationAdapterList          m_analysisSynapses;

    QMap<FilterPtr, struct Data> m_executingTasks;
    QMap<FilterPtr, struct Data> m_finishedTasks;

    friend class AppositionSurfaceTool;
  };

} // namespace ESPINA

#endif// APPOSITION_SURFACE_PLUGIN_H
