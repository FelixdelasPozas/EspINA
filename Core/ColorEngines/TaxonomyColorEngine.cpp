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

#include "Core/Model/Segmentation.h"
#include "Core/Model/Taxonomy.h"

using namespace EspINA;

const double SELECTED_ALPHA = 1.0;
const double UNSELECTED_ALPHA = 0.6;

//-----------------------------------------------------------------------------
QColor TaxonomyColorEngine::color(SegmentationPtr seg)
{
  if (seg && seg->taxonomy())
    return seg->taxonomy()->color();
  else
    return Qt::red;
}

//-----------------------------------------------------------------------------
LUTPtr TaxonomyColorEngine::lut(SegmentationPtr seg)
{
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  QString lutName;
  if (seg && seg->taxonomy())
    lutName = seg->taxonomy()->qualifiedName();

  vtkSmartPointer<vtkLookupTable> seg_lut;

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

//-----------------------------------------------------------------------------
void TaxonomyColorEngine::updateTaxonomyColor(TaxonomyElementPtr tax)
{
  QString lutName = tax->qualifiedName();
  QColor c = tax->color();

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