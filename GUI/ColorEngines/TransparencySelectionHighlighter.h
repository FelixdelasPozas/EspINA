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

#ifndef ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H
#define ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H

// ESPINA
#include <GUI/ColorEngines/SelectionHighlighter.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class EspinaGUI_EXPORT TransparencySelectionHighlighter
      : public SelectionHighlighter
      {
        public:
          virtual ~TransparencySelectionHighlighter()
          {};

          virtual QColor color(const QColor &original, bool highlight = false);
      };
    } // namespace ColorEngines
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H
