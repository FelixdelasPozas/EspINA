/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_UTILS_EVENTUTILS_H_
#define GUI_UTILS_EVENTUTILS_H_

#include <GUI/EspinaGUI_Export.h>

class QString;
class QEvent;

namespace ESPINA
{
  namespace GUI
  {
    namespace Utils
    {
      /** \brief Returns the type of the event as a string.
       * NOTE: If we ever update to Qt >= 5.5 this becomes irrelevant due
       *       to the existence of Q_ENUM utils in newer Qt versions.
       */
      QString EspinaGUI_EXPORT toString(QEvent *e);
    } // namespace Utils
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_UTILS_EVENTUTILS_H_
