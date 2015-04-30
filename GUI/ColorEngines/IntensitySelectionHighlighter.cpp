/*

 Copyright (C) {year} Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>

// VTK
#include <vtkMath.h>

using namespace ESPINA::GUI::ColorEngines;

//-----------------------------------------------------------------------------
QColor IntensitySelectionHighlighter::color(const QColor& original,
                                             bool highlight)
{
  auto suggestedColor = original;

  if (highlight)
  {
    double rgb[3]{original.redF(), original.greenF(), original.blueF()};
    double hsv[3];
    vtkMath::RGBToHSV(rgb, hsv);
    hsv[2] = 1.0;
    vtkMath::HSVToRGB(hsv, rgb);
    
    suggestedColor.setRedF(rgb[0]);
    suggestedColor.setGreenF(rgb[1]);
    suggestedColor.setBlueF(rgb[2]);
  }

  return suggestedColor;
}

//-----------------------------------------------------------------------------
QColor ESPINA::GUI::ColorEngines::defaultColor(const Hue color)
{
  QColor qcolor;
  qcolor.setHsv(color, 255, 150);

  return qcolor;
}

//-----------------------------------------------------------------------------
QColor ESPINA::GUI::ColorEngines::selectedColor(const Hue color)
{
  QColor qcolor;
  qcolor.setHsv(color, 255, 255);

  return qcolor;
}
