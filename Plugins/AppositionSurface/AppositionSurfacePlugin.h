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
#include <Support/Plugin.h>
#include <Core/Analysis/Input.h>
#include <Core/Analysis/DataFactory.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Types.h>

namespace ESPINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfacePlugin
  : public Support::Plugin
  {
    Q_OBJECT
    Q_INTERFACES(ESPINA::Support::Plugin)

    class ASFilterFactory
    : public FilterFactory
    {
        virtual FilterTypeList providedFilters() const;
        virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);
    };

  public:
    explicit AppositionSurfacePlugin();
    virtual ~AppositionSurfacePlugin();

    virtual void init(Support::Context &context);

    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    virtual QList<Support::CategorizedTool> tools() const;

    virtual Support::ReportSList reports() const;

    virtual Support::Settings::SettingsPanelSList settingsPanels() const;

    virtual FilterFactorySList filterFactories() const;

    static bool isValidSynapse(SegmentationAdapterPtr segmentation);

    static SegmentationAdapterPtr segmentationSAS(SegmentationAdapterPtr segmentation);

  public slots:
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


  private:
    Support::Context                *m_context;
    Support::Settings::SettingsPanelSPtr m_settings;
    SegmentationExtensionFactorySPtr m_extensionFactory;
    FilterFactorySPtr                m_filterFactory;
    //bool                             m_delayedAnalysis;
    SegmentationAdapterList          m_analysisSynapses;

    QMap<FilterPtr, struct Data> m_executingTasks;
    QMap<FilterPtr, struct Data> m_finishedTasks;

    friend class AppositionSurfaceTool;
  };

} // namespace ESPINA

#endif// APPOSITION_SURFACE_PLUGIN_H
