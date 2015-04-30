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

#ifndef GUI_COLORENGINES_SELECTIONHIGHLIGHTER_H_
#define GUI_COLORENGINES_SELECTIONHIGHLIGHTER_H_

// ESPINA
#include <GUI/ColorEngines/ColorEngine.h>

// Qt
#include <QMap>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class EspinaGUI_EXPORT SelectionHighlighter
      {
        public:
          /** \brief SelectionHighliter class virtual destructor.
           *
           */
          virtual ~SelectionHighlighter()
          {};

          /** \brief Returns the given color modified if the highlight flag is true.
           * \param[in] original, original color.
           * \param[in] highlight, true to highlight color false otherwise.
           *
           */
          virtual QColor color(const QColor &original, bool highlight = false) = 0;

          /** \brief Returns a LUT associated with the given color with two values,
           * the color and the transparent background.
           *
           */
          LUTSPtr lut(const QColor &original, bool highlight = false);

        private:
          /** \brief Generates and returns a unique key for the given color.
           * \param[in] color, color to generate a key.
           *
           */
          QString colorKey(const QColor &color) const;

        private:
          static ColorEngine::LUTMap m_LUT;
      };
    } // namespace ColorEngines
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_COLORENGINES_SELECTIONHIGHLIGHTER_H_
