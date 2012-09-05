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

#include "common/model/Segmentation.h"

const double SELECTED_ALPHA = 1.0;
const double UNSELECTED_ALPHA = 0.6;

vtkSmartPointer<vtkLookupTable> NumberColorEngine::lut(const Segmentation* seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  QString lutName = QString::number(seg->number());
  if (seg->isSelected())
    lutName.append("_selected");

  vtkSmartPointer<vtkLookupTable> seg_lut;

  bool updateLut = false;

  if (!m_LUT.contains(lutName))
  {
    double alpha = (seg->isSelected() ? SELECTED_ALPHA : UNSELECTED_ALPHA);
    QColor c = color(seg);

    seg_lut = vtkLookupTable::New();
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

QColor NumberColorEngine::color(const Segmentation* seg)
{
  if (seg)
    return QColor((seg->number() * 25) % 255, (seg->number() * 73) % 255, (seg->number() * 53) % 255);
  else
    return Qt::red;
}

