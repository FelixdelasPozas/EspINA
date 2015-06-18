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

// ESPINA
#include <Core/Utils/BlockTimer.h>

// Qt
#include <QDebug>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      
      BlockTimer::BlockTimer(const QString &name)
      : m_name{name}
      , m_startTime{std::chrono::high_resolution_clock::now()}
      {
      }
      
      BlockTimer::~BlockTimer()
      {
        auto endTime = std::chrono::high_resolution_clock::now();

        qDebug() << m_name << "execution time" <<  std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count() << "milliseconds.";
      }
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA
