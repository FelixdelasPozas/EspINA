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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H
#define ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H

// Qt
#include <QWidget>

// ESPINA
#include <Filters/SeedGrowSegmentationFilter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>

class QUndoStack;

namespace ESPINA {

  class RestrictToolGroup;

  class ROISettings;

  namespace Ui
  {
    class SeedGrowSegmentationHistoryWidget;
  }

  class SeedGrowSegmentationHistoryWidget
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationHistoryWidget(SeedGrowSegmentationFilterSPtr filter,
                                               RestrictToolGroup             *roiTools,
                                               Support::Context        &context);
    virtual ~SeedGrowSegmentationHistoryWidget();

  public slots:
    void setThreshold(int value);
    void setApplyClosing(bool value);
    void setClosingRadius(int value);

  signals:
    void thresholdChanged(int);
    void applyClosingChanged(bool);
    void closingRadiusChanged(int);

  private slots:
    void onThresholdChanged(int value);
    void onApplyClosingChanged(bool value);
    void onClosingRadiusChanged(int value);
    void onROIChanged();
    void onDiscardROIModifications();
    void modifyFilter();

  private:
    Support::Context &m_context;

    Ui::SeedGrowSegmentationHistoryWidget *m_gui;
    SeedGrowSegmentationFilterSPtr         m_filter;

    RestrictToolGroup *m_roiTools;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H
