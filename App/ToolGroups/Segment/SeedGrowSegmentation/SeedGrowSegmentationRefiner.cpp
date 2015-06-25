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

#include "SeedGrowSegmentationRefiner.h"
#include "SeedGrowSegmentationHistoryWidget.h"

#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <Settings/ROI/ROISettings.h>

using namespace ESPINA;
using namespace ESPINA::Support;

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::RefineWidget::RefineWidget(SegmentationAdapterPtr segmentation, Context& context)
: Count(0)
, RoiSettings(new ROISettings())
, RoiTools   (new RestrictToolGroup(RoiSettings, context))
{
  QColor sgsROIColor{Qt::yellow};
  sgsROIColor.setHslF(sgsROIColor.hueF(),sgsROIColor.saturationF(), 0.9);

  RoiTools->setColor(sgsROIColor);

  auto filter = dynamic_cast<SeedGrowSegmentationFilter *>(segmentation->filter().get());

  auto currentFilterROI = filter->roi();
  if (currentFilterROI)
  {
    RoiTools->setCurrentROI(currentFilterROI->clone());
  }
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::RefineWidget::~RefineWidget()
{
  RoiTools->setVisible(false);

  delete RoiTools;
  delete RoiSettings;
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::SeedGrowSegmentationRefiner()
{}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::~SeedGrowSegmentationRefiner()
{
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationRefiner::createWidget(SegmentationAdapterPtr segmentation, Context& context)
{
  if (!m_refineWidgets.contains(segmentation))
  {
    m_refineWidgets.insert(segmentation, new RefineWidget(segmentation, context));
  }

  auto rw = m_refineWidgets[segmentation];

  rw->Count++;
  rw->RoiTools->setVisible(true);

  auto filter = std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(segmentation->filter());
  auto widget = new SeedGrowSegmentationHistoryWidget(segmentation, filter, rw->RoiTools, context);

  connect(widget, SIGNAL(destroyed(QObject*)),
          this,   SLOT(onWidgetDestroyed(QObject *)));

//   connect(widget, SIGNAL(thresholdChanged(int)),
//           this,   SIGNAL(thresholdChanged(int)));
//   connect(this,   SIGNAL(thresholdChanged(int)),
//           widget, SLOT(setThreshold(int)));
//
//   connect(widget, SIGNAL(applyClosingChanged(bool)),
//           this,   SIGNAL(applyClosingChanged(bool)));
//
//   connect(this,   SIGNAL(applyClosingChanged(bool)),
//           widget, SLOT(setApplyClosing(bool)));
//
//   connect(widget, SIGNAL(closingRadiusChanged(int)),
//           this,   SIGNAL(closingRadiusChanged(int)));
//   connect(this,   SIGNAL(closingRadiusChanged(int)),
//           widget, SLOT(setClosingRadius(int)));

  m_widgetSegmentation[widget] = segmentation;

  return widget;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationRefiner::onWidgetDestroyed(QObject *widget)
{
  auto segmentation = m_widgetSegmentation[widget];
  auto refineWidget = m_refineWidgets[segmentation];

  refineWidget->Count--;

  if (0 == refineWidget->Count)
  {
    delete refineWidget;
    m_refineWidgets.remove(segmentation);
  }

  m_widgetSegmentation.remove(widget);
}