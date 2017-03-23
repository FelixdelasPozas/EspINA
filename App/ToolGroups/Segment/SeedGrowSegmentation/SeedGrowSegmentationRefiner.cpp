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

// ESPINA
#include "SeedGrowSegmentationRefiner.h"
#include "SeedGrowSegmentationRefineWidget.h"
#include <Settings/ROI/ROISettings.h>
#include <ToolGroups/Restrict/RestrictToolGroup.h>

using namespace ESPINA;
using namespace ESPINA::Support;

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::RefineWidget::RefineWidget(SegmentationAdapterPtr segmentation, Context& context)
: Count   {0}
, RoiTools{SeedGrowSegmentationRefiner::tools(context)}
{
  QColor sgsROIColor{Qt::yellow};
  sgsROIColor.setHslF(sgsROIColor.hueF(),sgsROIColor.saturationF(), 0.9);

  RoiTools->setColor(sgsROIColor);
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::RefineWidget::~RefineWidget()
{
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::SeedGrowSegmentationRefiner()
{
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationRefiner::~SeedGrowSegmentationRefiner()
{
}

//-----------------------------------------------------------------------------
RestrictToolGroupSPtr SeedGrowSegmentationRefiner::tools(Context &context)
{
  static RestrictToolGroupSPtr tools = std::make_shared<RestrictToolGroup>(new ROISettings(), context);

  return tools;
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationRefiner::createWidget(SegmentationAdapterPtr segmentation, Context& context, QWidget *parent)
{
  if (!m_refineWidgets.contains(segmentation))
  {
    m_refineWidgets.insert(segmentation, new RefineWidget(segmentation, context));
  }

  auto rw = m_refineWidgets[segmentation];

  rw->Count++;

  auto widget = new SeedGrowSegmentationRefineWidget(segmentation, rw->RoiTools, context, parent);

  connect(widget, SIGNAL(destroyed(QObject*)),
          this,   SLOT(onWidgetDestroyed(QObject *)));

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
