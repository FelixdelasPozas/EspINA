/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "CategoryColorEngine.h"
#include <GUI/Model/CategoryAdapter.h>

using namespace ESPINA;

const double SELECTED_ALPHA = 1.0;
const double UNSELECTED_ALPHA = 0.6;

//-----------------------------------------------------------------------------
QColor CategoryColorEngine::color(SegmentationAdapterPtr seg)
{
  QColor color(Qt::red);

  if (seg && seg->category())
  {
    color = seg->category()->color();
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr CategoryColorEngine::lut(SegmentationAdapterPtr seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  QString lutName;
  if (seg && seg->category())
    lutName = seg->category()->classificationName();

  LUTSPtr seg_lut = nullptr;

  if (!m_LUT.contains(lutName))
  {
    auto alpha = (seg->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA);
    auto c = color(seg);

    seg_lut = LUTSPtr::New();
    seg_lut->Allocate();
    seg_lut->SetNumberOfTableValues(2);
    seg_lut->Build();
    seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
    seg_lut->Modified();

    m_LUT.insert(lutName, seg_lut);

    // TODO 2015-04-20: Check signals
    if (lutName != "")
      connect(seg->category().get(), SIGNAL(colorChanged(CategoryElementPtr)),
              this,                  SLOT(updateCategoryColor(CategoryElementPtr)));
  }
  else
  {
    // TODO 2015-04-20:sometimes happens, segs without category, fixed?
    // fix a corner case when a segmentation and it's category have been deleted
    // but the lookuptable hasn't, so when the segmentation and category are
    // created again with a different color the ColorEngine returns the lookuptable
    // with the old color.
    Q_ASSERT(false);
    if (seg->category())
    {
      double rgb[3];
      m_LUT[lutName]->GetColor(1, rgb);
      auto segColor = seg->category()->color();

      if (segColor != QColor(rgb[0], rgb[1], rgb[2]))
        m_LUT[lutName]->SetTableValue(1, segColor.redF(), segColor.greenF(), segColor.blueF(), (seg->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA));

      seg_lut = m_LUT[lutName];
    }
  }

  return seg_lut;
}

//-----------------------------------------------------------------------------
void CategoryColorEngine::updateCategoryColor(CategoryAdapterSPtr category)
{
  auto lutName = category->classificationName();
  auto c = category->color();

  if (!m_LUT.contains(lutName))
    return;

  m_LUT[lutName]->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), UNSELECTED_ALPHA);
  m_LUT[lutName]->Modified();

  lutName.append("_selected");

  if (!m_LUT.contains(lutName))
    return;

  m_LUT[lutName]->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), SELECTED_ALPHA);
  m_LUT[lutName]->Modified();
}
