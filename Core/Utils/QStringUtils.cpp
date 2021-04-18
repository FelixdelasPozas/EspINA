/*
 File: QStringUtils.cpp
 Created on: 30/01/2021
 Author: Felix de las Pozas Alvarez

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

// EspINA
#include "QStringUtils.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
QString Core::Utils::simplifyString(const QString &s)
{
  QString cleanedName;
  auto cleanChar = [&cleanedName](const QChar &c)
  {
    auto decomposition = c.decomposition();
    if(!decomposition.isEmpty())
    {
      cleanedName.append(decomposition.at(0));
    }
    else
    {
      cleanedName.append(c);
    }
  };
  std::for_each(s.cbegin(), s.cend(), cleanChar);

  return cleanedName;
}
