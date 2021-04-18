/*
 File: QStringUtils.h
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

#ifndef CORE_UTILS_QSTRINGUTILS_H_
#define CORE_UTILS_QSTRINGUTILS_H_

#include "Core/EspinaCore_Export.h"

// Qt
#include <QString>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \brief Returns a string with only ASCII valid characters.
       * \param[in] s QString reference.
       *
       */
      QString EspinaCore_EXPORT simplifyString(const QString &s);
    }
  }
}
#endif // CORE_UTILS_QSTRINGUTILS_H_
