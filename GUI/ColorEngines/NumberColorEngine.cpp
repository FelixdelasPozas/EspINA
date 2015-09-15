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
#include "NumberColorEngine.h"
#include <GUI/Model/SegmentationAdapter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

const double SELECTED_ALPHA   = 1.0;
const double UNSELECTED_ALPHA = 0.6;

//-----------------------------------------------------------------------------
NumberColorEngine::NumberColorEngine()
: ColorEngine("NumberColorEngine", tr("Segmentation"))
{

}

//-----------------------------------------------------------------------------
QColor NumberColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  int h = 359;
  int s = 255;
  int v = 255;

  if (segmentation)
  {
    const int HUE_SHIFT = 41;
    h = (segmentation->number() * HUE_SHIFT) % 360;
  }

  return QColor::fromHsv(h, s, v);
}

//-----------------------------------------------------------------------------
LUTSPtr NumberColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  auto lutName = QString::number(segmentation->number());

  LUTSPtr seg_lut = nullptr;

  if (!m_LUT.contains(lutName))
  {
    auto alpha = (segmentation->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA);
    auto c = color(segmentation);

    seg_lut = LUTSPtr::New();
    seg_lut->Allocate();
    seg_lut->SetNumberOfTableValues(2);
    seg_lut->Build();
    seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
    seg_lut->Modified();

    m_LUT.insert(lutName, seg_lut);
  }
  else
    seg_lut = m_LUT[lutName];

  return seg_lut;
}
