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

#include "Conditions.h"

using namespace ESPINA;

QString ESPINA::condition(const QString &icon, const QString &description)
{
  return QString("<table style=\"margin: 0px\">"
               " <tr>"
               " <td valign=\"top\"><img src='%1' width=16 height=16></td> <td valign=\"center\">: %2</td>"
               " </tr>"
               "</table>"
              ).arg(icon).arg(description);
}
