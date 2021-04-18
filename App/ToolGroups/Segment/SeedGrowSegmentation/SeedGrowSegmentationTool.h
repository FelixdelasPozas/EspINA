/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
#define ESPINA_SEED_GROW_SEGMENTATION_TOOL_H

// ESPINA
#include "SeedThreshold.h"
#include "CustomROIWidget.h"
#include "SeedGrowSegmentationRefineWidget.h"
#include <Filters/SeedGrowSegmentationFilter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Selectors/Selector.h>
#include <Support/Widgets/ProgressTool.h>
#include <GUI/Types.h>
#include <Support/Factory/FilterRefinerFactory.h>

class QCheckBox;
class QUndoStack;
class QPushButton;

namespace ESPINA
{
  class SeedGrowSegmentationSettings;

  /** \class SeedGrowSegmentationRefiner
   * \brief Filter refiner widget factory for SGS filters.
   *
   */
  class SeedGrowSegmentationRefiner
  : public FilterRefiner
  {
    Q_OBJECT
    public:
      virtual QWidget* createWidget(SegmentationAdapterPtr segmentation, Support::Context& context, QWidget *parent = nullptr)
      {
        return new SeedGrowSegmentationRefineWidget(segmentation, context, parent);
      }
  };

  /** \class SeedGrowSegmentationFactory
   * \brief Factory for seed grow segmentation filters.
   *
   */
  class SeedGrowSegmentationFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SGS_FILTER;    /** seed grow filter signature. */
      static const Filter::Type SGS_FILTER_V4; /** seed grow filter old signature. */

      virtual const FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory of this provider. */
  };

  /** \class SeedGrowSegmentationTool
   * \brief Tool for seed grow segmentation filter (now grey level segmentation).
   *
   */
  class SeedGrowSegmentationTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief SeedGrowSegmentationTool class constructor.
       * \param[in] settings raw pointer to a SeedGrowSegmentationSettings object.
       * \param[in] filterRefiners register of filter refiners
       * \param[in] context of current session
       */
      explicit SeedGrowSegmentationTool(SeedGrowSegmentationSettings   *settings,
                                        Support::FilterRefinerFactory &filterRefiners,
                                        Support::Context               &context);

      /** \brief SeedGrowSegmentation class virtual destructor.
       *
       */
      virtual ~SeedGrowSegmentationTool();

      virtual void abortOperation() override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    private:
      /** \brief Helper to initialize both pixel selectors.
       *
       */
      void initPixelSelectors();

      /** \brief Helper method to initalize the normal pixel selector.
       *
       */
      void initPixelSelector();

      /** \brief Helper method to initalize the best pixel selector.
       *
       */
      void initBestPixelSelector();

      /** \brief Helper method to configure a selector and connect it's signals.
       *
       */
      void initSelector(SelectorSPtr selector);

      /** \brief Helper method to initialize the settings widgets.
       *
       */
      void initSettingsWidgets();

      /** \brief Helper method to initialize the category selector widget.
       *
       */
      void initCategorySelector();

      /** \brief Helper method to initialize the ROI widgets.
       *
       */
      void initROISelector();

      /** \brief Helper method to initialize the best pixel range selector.
       *
       */
      void initBestPixelWidgets();

      /** \brief Helper method to initialize the close morphological operation widget.
       *
       */
      void initCloseWidgets();

      /** \brief Returns the input channel for the tool.
       *
       */
      ChannelAdapterPtr inputChannel() const;

      /** \brief Returns the current active pixel selector.
       *
       */
      SelectorSPtr activeSelector() const;

      /** \brief Aborts all executing tasks.
       *
       */
      void abortTasks();

    private slots:
      /** \brief Launches a seedgrow segmentation task based on the current selection.
       * \pararm[in] selectedItems, current selection.
       *
       */
      void launchTask(Selector::Selection selectedItems);

      /** \brief After the filter has finished adds the segmentation to the model.
       *
       */
      void createSegmentation();

      /** \brief Updates the ROI values when the category changes.
       * \param[in] category current category.
       *
       */
      void onCategoryChanged(CategoryAdapterSPtr category);

      /** \brief Updates ROI values.
       * \param[in] update true if category ROI values have to be applied.
       *
       */
      void updateCurrentCategoryROIValues(bool update);

      void useBestPixelSelector(bool value);

      /** \brief Updated the best pixel value.
       * \param[in] value integer value in range [0-254]
       *
       */
      void onNewPixelValue(int value);

      /** \brief Updates the UI when the close check changes value.
       * \param[in] value boolean value.
       *
       */
      void onCloseStateChanged(bool value);

      /** \brief Updates the settings when the user changes the close radius value.
       * \param[in] value New radius value.
       */
      void onRadiusValueChanged(int value);

    private:
      using CategorySelector   = GUI::Widgets::CategorySelector;
      using PixelValueSelector = GUI::Widgets::PixelValueSelector;
      using NumericalInput     = GUI::Widgets::NumericalInput;

      CategorySelector   *m_categorySelector; /** category selector widget.     */
      SeedThreshold      *m_seedThreshold;    /** seed threshold input widget.  */
      QPushButton        *m_useBestPixel;     /** best pixel selection button.  */
      QLabel             *m_colorLabel;       /** best pixel color label.       */
      PixelValueSelector *m_colorSelector;    /** best pixel color selector.    */
      CustomROIWidget    *m_roi;              /** ROI widget.                   */
      QPushButton        *m_applyClose;       /** appy close selection button.  */
      NumericalInput     *m_close;            /** close radius selector button. */

      SeedGrowSegmentationSettings* m_settings; /** tool's settings. */

      SelectorSPtr m_bestPixelSelector; /** best pixel selector.         */
      SelectorSPtr m_pixelSelector;     /** normal pixel selector.       */
      SelectorSPtr m_activeSelector;    /** currently selected selector. */

      std::shared_ptr<SeedGrowSegmentationFilterFactory>  m_sgsFactory; /** tool's filter factory. */

      /** \struct Data
       * \brief Context data for tool execution.
       *
       */
      struct Data
      {
        SeedGrowSegmentationFilterSPtr Filter;   /** SGS filter.                      */
        CategoryAdapterSPtr            Category; /** created segmentation's category. */
      };

      QMap<FilterPtr, struct Data> m_tasks; /** maps filter<->data. */
  };

  using SeedGrowSegmentationToolSPtr = std::shared_ptr<SeedGrowSegmentationTool>;

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
