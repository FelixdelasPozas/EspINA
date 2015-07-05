/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
QColor IntensitySelectionHighlighter::color(const QColor& color, bool highlight)
{
  return QColor::fromHsvF(color.hsvHueF(),
                          color.hsvSaturationF(),
                          highlight?1.0:qMin(color.valueF(), 0.8),
                          color.alphaF());
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
