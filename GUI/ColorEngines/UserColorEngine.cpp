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
#include "UserColorEngine.h"
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkColorTransferFunction.h>
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

const double SELECTED_ALPHA = 1.0;
const double UNSELECTED_ALPHA = 0.6;

//-----------------------------------------------------------------------------
UserColorEngine::UserColorEngine()
: ColorEngine("UserColorEngine", tr("Color by user that created the segmentation."))
, m_lastColor(0)
{
  m_colors << QColor( 31, 120, 180)
           << QColor( 51, 160,  44)
           << QColor(227,  26,  28)
           << QColor(255, 127,   0)
           << QColor(106,  61, 154)
           << QColor(166, 206, 227)
           << QColor(178, 223, 138)
           << QColor(251, 154, 153)
           << QColor(253, 191, 111)
           << QColor(202, 178, 214);
}

//-----------------------------------------------------------------------------
QColor UserColorEngine::color(ConstSegmentationAdapterPtr seg)
{
  auto user = seg->users().last();

  if (!m_userColors.contains(user))
  {
    m_userColors[user] = nextColor();
  }

  return m_userColors[user];
}

//-----------------------------------------------------------------------------
LUTSPtr UserColorEngine::lut(ConstSegmentationAdapterPtr seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  auto lutName = seg->users().join("");

  LUTSPtr seg_lut = nullptr;

  if (m_LUT.find(lutName) == m_LUT.end())
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
  }
  else
  {
    Q_ASSERT(false);
    // fix a corner case when a segmentation and it's user entry have been deleted
    // but the lookuptable hasn't, so when the segmentation and user are been
    // created again with a different color the ColorEngine returns the lookuptable
    // with the old color.
    double rgb[3];
    m_LUT[lutName]->GetColor(1, rgb);
    auto segColor = seg->category()->color();

    if (segColor != QColor(rgb[0], rgb[1], rgb[2]))
    {
      m_LUT[lutName]->SetTableValue(1, segColor.redF(), segColor.greenF(), segColor.blueF(), (seg->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA));
    }

    seg_lut = m_LUT.find(lutName).value();
  }

  return seg_lut;
}


//-----------------------------------------------------------------------------
QColor UserColorEngine::nextColor()
{
  return m_colors[++m_lastColor%m_colors.size()];
}
