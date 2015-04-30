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

#ifndef GUI_COLORENGINES_INTENSITYSELECTIONHIGHTLIGHTER_H_
#define GUI_COLORENGINES_INTENSITYSELECTIONHIGHTLIGHTER_H_

#include "EspinaGUI_Export.h"

// ESPINA
#include <GUI/ColorEngines/SelectionHighlighter.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      
      class EspinaGUI_EXPORT IntensitySelectionHighlighter
      : public SelectionHighlighter
      {
        public:
          virtual ~IntensitySelectionHighlighter()
          {};

          virtual QColor color(const QColor &original, bool highlight = false);
      };

      QColor defaultColor(const Hue color);

      QColor selectedColor(const Hue color);
    
    } // namespace ColorEngines
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_COLORENGINES_INTENSITYSELECTIONHIGHTLIGHTER_H_
