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

QColor TaxonomyColorEngine::color(Segmentation* seg)
{
  if (seg && seg->taxonomy())
    return seg->taxonomy()->color();
  else
    return Qt::red;
}

vtkSMProxy *TaxonomyColorEngine::lut(Segmentation* seg)
{
//   // Get (or create if it doesn't exit) the lut for the segmentations' images
//   pqServer *server =  pqApplicationCore::instance()->getActiveServer();
//   QString lutName = m_seg->taxonomy()->qualifiedName();
//   if (m_seg->isSelected())
//     lutName.append("_selected");
// 
//   m_LUT = pqApplicationCore::instance()->getLookupTableManager()->getLookupTable(server,lutName,4,0);
//   if (m_LUT)
//   {
//     vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
//       m_LUT->getProxy()->GetProperty("RGBPoints"));
//     if (rgbs)
//     {
// 
//       double color[4];
//       double rgba[4];
//       rgba[3] = 1;
//       m_seg->color(color);
//       bool isSelected = m_seg->isSelected();
//       for(int c=0; c<3; c++)
//       {
// 	rgba[c] = color[c]*(isSelected?1:0.7);
//       }
//       double colors[8] = {0,0,0,0,1,rgba[0],rgba[1],rgba[2]};
//       rgbs->SetElements(colors);
//     }
//     m_LUT->getProxy()->UpdateVTKObjects();
//   }
  return NULL;
}


