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
#include <Filters/SeedGrowSegmentationFilter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Widgets/CategorySelector.h>
#include <Support/Factory/FilterDelegateFactory.h>
#include <Support/ViewManager.h>
#include <Support/Widgets/Tool.h>

class QUndoStack;
namespace ESPINA
{
  class SeedGrowSegmentationSettings;

  class SeedGrowSegmentationTool
  : public Tool
  {
    Q_OBJECT

    class SGSFactory
    : public FilterFactory
    , public SpecificFilterDelegateFactory
    {
      /** \brief Implements FilterFactory::providedFilters().
       *
       */
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      virtual QList<Filter::Type> availableFilterDelegates() const;

      virtual FilterDelegateSPtr createDelegate(FilterSPtr filter) throw (Unknown_Filter_Type_Exception);

    private:
      mutable DataFactorySPtr m_dataFactory;
    };

  public:
    /** \brief SeedGrowSegmentationTool class constructor.
     * \param[in] settings raw pointer to a SeedGrowSegmentationSettings object.
     * \param[in] model model adapter smart pointer.
     * \param[in] factory factory smart pointer.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] undoStack raw pointer to a QUndoStack object.
     */
    explicit SeedGrowSegmentationTool(SeedGrowSegmentationSettings* settings,
                                      ModelAdapterSPtr              model,
                                      ModelFactorySPtr              factory,
                                      FilterDelegateFactorySPtr     filterDelegateFactory,
                                      ViewManagerSPtr               viewManager,
                                      QUndoStack*                   undoStack);

    /** \brief SeedGrowSegmentation class virtual destructor.
     *
     */
    virtual ~SeedGrowSegmentationTool();

    virtual QList<QAction *> actions() const;

  private:
    virtual void onToolEnabled(bool enabled);

    /** \brief Adds a selector to the list of selectors.
     * \param[in] action QAction object raw pointer to add as selector action.
     * \param[in] selector selector smart pointer to add.
     *
     */
    void addVoxelSelector(QAction *action, SelectorSPtr selector);

  private slots:
  /** \brief Changes the current selector.
   * \param[in] action action associated to the selector.
   *
   */
    void changeSelector(QAction *action);

    /** \brief Unsets the selector.
     *
     */
    void unsetSelector();

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

    /** \brief Updates the ROI values on category widget creation.
     *
     */
    void onCategorySelectorWidgetCreation();

    /** \brief Updates ROI values.
     * \param[in] update true if category ROI values have to be applied.
     *
     */
    void updateCurrentCategoryROIValues(bool update);

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    CategorySelector *m_categorySelector;
    ActionSelector   *m_selectorSwitch;
    SeedThreshold    *m_seedThreshold;
    CustomROIWidget  *m_roi;

    SeedGrowSegmentationSettings* m_settings;

    QMap<QAction *, SelectorSPtr> m_voxelSelectors;
    SelectorSPtr                  m_currentSelector;

    std::shared_ptr<SGSFactory>  m_sgsFactory;

    QMap<FilterPtr, FilterSPtr> m_executingTasks;
    QMap<FilterPtr, SeedGrowSegmentationFilterSPtr> m_executingFilters;
  };

  using SeedGrowSegmentationToolSPtr = std::shared_ptr<SeedGrowSegmentationTool>;

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
