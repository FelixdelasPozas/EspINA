/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_GUI_UTILS_FORMAT_H
#define ESPINA_GUI_UTILS_FORMAT_H

#include <GUI/EspinaGUI_Export.h>

// Qt
#include <QString>

namespace ESPINA
{
  namespace GUI
  {
    namespace Utils
    {
      namespace Format
      {
        /** \brief Returns the tooltip text for the given icon and description.
         * \param[in] icon of the condition
         * \param[in] description of the condition
         *
         */
        inline QString createTable(const QString &icon, const QString &description)
        {
          return QString("<table cellspacing='0' cellpadding='0' border-spacing='0' style=\"margin:0;border:0;padding:0\">"
                         " <tr>"
                         " <td valign=\"top\"><img src='%1' width=16 height=16></td> <td valign=\"center\" height=0>: %2</td>"
                         " </tr>"
                         "</table>").arg(icon).arg(description);
        }

        /** \brief Returns a link to the given reference for the given label.
         * \param[in] label text.
         * \param[in] reference destination of the link.
         *
         */
        inline QString createLink(const QString &label, const QString &reference)
        {
          return QString("<a href=\"%1\">%2</a> ").arg(reference).arg(label);
        }

        /** \brief Returns a link for the given label.
         * \param[in] label text.
         *
         */
        inline QString createLink(const QString &label)
        {
          return createLink(label, label);
        }
      };
    }
  }
}

#endif // ESPINA_GUI_UTILS_FORMAT_H
