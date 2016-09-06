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

#ifndef CORE_UTILS_BLOCKTIMER_H_
#define CORE_UTILS_BLOCKTIMER_H_

#include <Core/EspinaCore_Export.h>

// Qt
#include <QString>

// C++
#include <chrono>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class BlockTimer
       * \brief Timer for the execution of source code blocks. Counts milliseconds from object creation to
       * destruction.
       */
      class EspinaCore_EXPORT BlockTimer
      {
        public:
          /** \brief BlockTimer class constructor.
           * \param[in] id timer id.
           */
          BlockTimer(const QString &id);

          /** \brief BlockTimer class destructor.
           *
           */
          ~BlockTimer();

        private:
          QString m_id; /** timer id. */
          std::chrono::high_resolution_clock::time_point m_startTime; /** timer creation time. */
      };
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_BLOCKTIMER_H_
