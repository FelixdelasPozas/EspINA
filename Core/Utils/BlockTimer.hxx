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

#ifndef CORE_UTILS_BLOCKTIMER_HXX_
#define CORE_UTILS_BLOCKTIMER_HXX_

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
      template<typename T = std::chrono::milliseconds>
      class BlockTimer
      {
        public:
          /** \brief BlockTimer class constructor.
           * \param[in] name timer id.
           */
          BlockTimer(const QString &name)
          : m_id       {name}
          , m_startTime{std::chrono::high_resolution_clock::now()}
          {}

          /** \brief BlockTimer class destructor.
           *
           */
          ~BlockTimer()
          {
            auto endTime = std::chrono::high_resolution_clock::now();

            qDebug() << m_id << "execution time" << std::chrono::duration_cast<T>(endTime - m_startTime).count() << printUnits();
          }

          /** \brief Returns the elapsed time from the object creation in the specified units.
           *
           */
          int elapsed() const
          {
            auto nowTime = std::chrono::high_resolution_clock::now();

            return std::chrono::duration_cast<T>(nowTime - m_startTime).count();
          }

        private:
          /** \brief Returns the adequate string for the templated time units.
           *
           */
          const QString printUnits()
          {
            if(T::period::num == 1)
            {
              if(T::period::den == std::nano::den)
              {
                return "nanoseconds";
              }
              else
              {
                if(T::period::den == std::micro::den)
                {
                  return "microseconds";
                }
                else
                {
                  if(T::period::den == std::milli::den)
                  {
                    return "milliseconds";
                  }
                  else
                  {
                    if(T::period::den == 1)
                    {
                      return "seconds";
                    }
                  }
                }
              }
            }
            else
            {
              if(T::period::den == 1)
              {
                if(T::period::num == 60)
                {
                  return "minutes";
                }
                else
                {
                  if(T::period::num == 3600)
                  {
                    return "hours";
                  }
                }
              }
            }

            return "unknown units";
          }

          QString m_id;                                               /** timer id.            */
          std::chrono::high_resolution_clock::time_point m_startTime; /** timer creation time. */
      };
    
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_BLOCKTIMER_HXX_
