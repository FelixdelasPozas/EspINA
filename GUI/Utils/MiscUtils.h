/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_UTILS_MISCUTILS_H_
#define GUI_UTILS_MISCUTILS_H_

// ESPINA
#include <GUI/EspinaGUI_Export.h>

class QPixmap;
class QString;

namespace ESPINA
{
  namespace GUI
  {
    namespace Utils
    {
      /** \brief Concatenates a pixmap to the given one and returns it.
       * \param[in] original Original pixmap.
       * \param[in] image Name of the pixmap to add.
       * \param[in] slim True to use the slim version of the pixmap.
       *
       */
      const QPixmap EspinaGUI_EXPORT appendImage(const QPixmap& original, const QString& image, bool slim = false);

    } // namespace Utils
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_UTILS_MISCUTILS_H_
