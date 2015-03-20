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

#include "SeedGrowSegmentationHistory.h"
#include "SeedGrowSegmentationHistoryWidget.h"

#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <Settings/ROI/ROISettings.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
SeedGrowSegmentationHistory::~SeedGrowSegmentationHistory()
{
  delete m_roiTools;
  delete m_roiSettings;
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationHistory::createWidget(ModelAdapterSPtr model, ModelFactorySPtr factory, ViewManagerSPtr viewManager, QUndoStack* undoStack)
{
  if (!m_roiTools)
  {
    m_roiSettings = new ROISettings();
    m_roiTools    = new RestrictToolGroup(m_roiSettings, model, factory, viewManager, undoStack);

    QColor sgsROIColor{Qt::yellow};
    sgsROIColor.setHslF(sgsROIColor.hueF(),sgsROIColor.saturationF(), 0.9);
    m_roiTools->setColor(sgsROIColor);

    auto currentFilterROI = m_filter->roi();
    if (currentFilterROI)
    {
      m_roiTools->setCurrentROI(currentFilterROI->clone());
    }
  }

  m_widgetCount++;
  m_roiTools->setVisible(true);

  auto widget = new SeedGrowSegmentationHistoryWidget(m_filter, m_roiTools, viewManager, undoStack);

  connect(widget, SIGNAL(destroyed(QObject*)),
          this,   SLOT(onWidgetDestroyed()));

  connect(widget, SIGNAL(thresholdChanged(int)),
          this,   SIGNAL(thresholdChanged(int)));
  connect(this,   SIGNAL(thresholdChanged(int)),
          widget, SLOT(setThreshold(int)));

  connect(widget, SIGNAL(applyClosingChanged(bool)),
          this,   SIGNAL(applyClosingChanged(bool)));
  connect(this,   SIGNAL(applyClosingChanged(bool)),
          widget, SLOT(setApplyClosing(bool)));

  connect(widget, SIGNAL(closingRadiusChanged(int)),
          this,   SIGNAL(closingRadiusChanged(int)));
  connect(this,   SIGNAL(closingRadiusChanged(int)),
          widget, SLOT(setClosingRadius(int)));

  return widget;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationHistory::onWidgetDestroyed()
{
  m_widgetCount--;

  if (0 == m_widgetCount)
  {
    m_roiTools->setVisible(false);

  }
}
