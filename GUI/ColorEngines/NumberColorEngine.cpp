/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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
#include "NumberColorEngine.h"

using namespace EspINA;

const double SELECTED_ALPHA   = 1.0;
const double UNSELECTED_ALPHA = 0.6;

//-----------------------------------------------------------------------------
QColor NumberColorEngine::color(SegmentationAdapterPtr segmentation)
{
  int r = 255;
  int g = 0;
  int b = 0;

  if (segmentation)
  {
    r = (segmentation->number() * 25) % 255;
    g = (segmentation->number() * 73) % 255;
    b = (segmentation->number() * 53) % 255;
  }

  return QColor(r, g, b);
}

//-----------------------------------------------------------------------------
LUTSPtr NumberColorEngine::lut(SegmentationAdapterPtr segmentation)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  QString lutName = QString::number(segmentation->number());
//   if (seg->isSelected())
//     lutName.append("_selected");

  LUTSPtr seg_lut;

  if (!m_LUT.contains(lutName))
  {
    double alpha = (segmentation->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA);
    QColor c = color(segmentation);

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
