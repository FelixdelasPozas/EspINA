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
  /** \class ASFilterFactory
   * \brief Factory for apposition surface filters.
   *
   */
  class AppositionSurfacePlugin_EXPORT ASFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type AS_FILTER; /** apposition surface filter signature. */

      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;
    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory of this provider. */
  };

  class AppositionSurfacePlugin_EXPORT AppositionSurfacePlugin
  : public Support::Plugin
  {
    Q_OBJECT
    Q_INTERFACES(ESPINA::Support::Plugin)

  public:
    /** \brief AppositionSurfacePlugin class constructor.
     *
     */
    explicit AppositionSurfacePlugin();

    /** \brief AppositionSurfacePlugin class virtual destructor.
     *
     */
    virtual ~AppositionSurfacePlugin();

    virtual void init(Support::Context &context);

    virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

    virtual QList<Support::CategorizedTool> tools() const;

    virtual Support::ReportSList reports() const;

    virtual Support::Settings::SettingsPanelSList settingsPanels() const;

    virtual FilterFactorySList filterFactories() const;

    /** \brief Returns true if the segmentation given is of a correct category.
     * \param[in] segmentation segmentation to check for correct category.
     *
     */
    static bool isValidSynapse(SegmentationAdapterPtr segmentation);

    /** \brief Returs the apposition surface segmentation corresponding to the given segmentation.
     * \param[in] segmentation to search for it's corresponding SAS.
     *
     */
    static SegmentationAdapterPtr segmentationSAS(SegmentationAdapterPtr segmentation);

  public slots:
    /** \brief Creates SAS for the newly created segmentations given.
     * \param[in] segmentations list of recently created segmentations.
     *
     */
    void segmentationsAdded(ViewItemAdapterSList segmentations);

    /** \brief Adds the created SAS to the model and creates SAS category if necessary.
     *
     */
    void finishedTask();

  private:
    /** \brief Returs true if the given item is a SAS.
     * \param[in] item view item to check.
     *
     */
    static bool isSAS(ItemAdapterSPtr item);

  private:
    /** \struct Data
     * \brief Filters data needed for execution.
     *
     */
    struct Data
    {
      FilterSPtr              adapter;      /** SAS filter. */
      SegmentationAdapterSPtr segmentation; /** originating segmentation. */

      /** \brief Data constructor.
       * \param[in] adapterP SAS filter.
       * \param[in] segmentation originating segmentation.
       *
       */
      Data(FilterSPtr adapterP, SegmentationAdapterSPtr segmentationP)
      : adapter{adapterP}, segmentation{segmentationP}
      {};

      /** \brief Data empty constructor.
       *
       */
      Data(): adapter{nullptr}, segmentation{nullptr}
      {};
    };


  private:
    Support::Context                    *m_context;          /** application context. */
    Support::Settings::SettingsPanelSPtr m_settings;         /** SAS execution settings. */
    SegmentationExtensionFactorySPtr     m_extensionFactory; /** segmentation extensions factory. */
    FilterFactorySPtr                    m_filterFactory;    /** filters factory. */

    QMap<FilterPtr, struct Data> m_executingTasks; /** task currenty in execution, maps filter-data. */
    QMap<FilterPtr, struct Data> m_finishedTasks;  /** finished tasks, maps filter-data. */

    friend class AppositionSurfaceTool;
  };

} // namespace ESPINA

#endif// APPOSITION_SURFACE_PLUGIN_H
