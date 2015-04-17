/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
#define ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H

#include <Support/FilterHistory.h>
#include <Filters/SeedGrowSegmentationFilter.h>


namespace ESPINA {

  class RestrictToolGroup;
  class ROISettings;

  class SeedGrowSegmentationHistory
  : public FilterHistory
  {
    Q_OBJECT
  public:
    SeedGrowSegmentationHistory(SeedGrowSegmentationFilterSPtr filter)
    : m_filter(filter)
    , m_widgetCount(0)
    , m_roiSettings(nullptr)
    , m_roiTools(nullptr)
    {}

    virtual ~SeedGrowSegmentationHistory();

    virtual QWidget *createWidget(Support::Context &context);

  signals:
    void thresholdChanged(int);
    void applyClosingChanged(bool);
    void closingRadiusChanged(int);

  private slots:
    /**
     *  Decrease widget count and hides ROI if no widgets are visible
     */
    void onWidgetDestroyed();

  private:
    SeedGrowSegmentationFilterSPtr m_filter;

    int m_widgetCount;

    ROISettings   *m_roiSettings;
    RestrictToolGroup *m_roiTools;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
