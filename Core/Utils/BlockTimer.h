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
      
      class BlockTimer
      {
        public:
          BlockTimer(const QString &name);
          virtual ~BlockTimer();

        private:
          QString m_name;
          std::chrono::high_resolution_clock::time_point m_startTime;
      };
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_BLOCKTIMER_H_
