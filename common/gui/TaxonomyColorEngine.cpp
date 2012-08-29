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

#include "TaxonomyColorEngine.h"

#include "common/model/Segmentation.h"

QColor TaxonomyColorEngine::color(const Segmentation* seg)
{
  if (seg && seg->taxonomy())
    return seg->taxonomy()->color();
  else
    return Qt::red;
}

vtkSmartPointer< vtkLookupTable > TaxonomyColorEngine::lut(const Segmentation* seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  QString lutName = seg->taxonomy()->qualifiedName();
  if (seg->isSelected())
    lutName.append("_selected");

  vtkSmartPointer<vtkLookupTable> seg_lut;

  if (m_LUT.find(lutName) == m_LUT.end())
  {
    double alpha = (seg->isSelected() ? 1.0 : 0.7);
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
    seg_lut = m_LUT.find(lutName).value();

  return seg_lut;
}

