/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <Support/Tool.h>
#include <Support/ViewManager.h>

#include "SeedThreshold.h"
#include "ApplyROI.h"
#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/ModelFactory.h>

class QUndoStack;
namespace EspINA
{
  class SeedGrowSegmentationTool
  : public Tool
  {
    class SGSFilterFactory
    : public FilterFactory
    {
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

    private:
      mutable FetchBehaviourSPtr m_fetchBehaviour;
    };

    Q_OBJECT
  public:
    explicit SeedGrowSegmentationTool(ModelAdapterSPtr model,
                                      ModelFactorySPtr factory,
                                      ViewManagerSPtr  viewManager,
                                      QUndoStack      *undoStack);
    virtual ~SeedGrowSegmentationTool();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual QList<QAction *> actions() const;

  private:
    void addVoxelSelector(QAction *action, SelectorSPtr selector);

  private slots:
    void changeSelector(QAction *action);

    void unsetSelector();

    void launchTask(Selector::SelectionList selectedItems);

    void onTaskProgres(int progress);

    void createSegmentation();

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    bool             m_enabled;

    CategorySelector *m_categorySelector;
    ActionSelector   *m_selectorSwitch;
    SeedThreshold    *m_seedThreshold;
    ApplyROI         *m_applyVOI;

    QMap<QAction *, SelectorSPtr> m_voxelSelectors;
    SelectorSPtr m_currentSelector;

    FilterFactorySPtr  m_filterFactory;
    QMap<FilterAdapterPtr, FilterAdapterSPtr> m_executingTasks;
  };

  using SeedGrowSegmentationToolSPtr = std::shared_ptr<SeedGrowSegmentationTool>;

} // namespace EspINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
