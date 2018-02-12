/*
 * Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#ifndef CORE_UTILS_BLOCKTIMER_HXX_
#define CORE_UTILS_BLOCKTIMER_HXX_

// C++
#include <chrono>

// Qt
#include <QString>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class BlockTimer
       * \brief Timer for the execution of source code blocks.
       *
       * \tparam TimeUnits std::chrono units (milliseconds is pretty formated).
       */
      template<typename TimeUnits = std::chrono::milliseconds>
      class BlockTimer
      {
        public:
          /** \brief BlockTimer class constructor.
           *
           * \param[in] name timer id.
           */
          BlockTimer(const QString &name);

          /** \brief BlockTimer class destructor.
           *
           */
          ~BlockTimer();

          /** \brief Returns the elapsed time from the object creation in the specified units.
           *
           */
          int elapsed() const;

          /** \brief Sets the initial time to current time
           *
           */
          void reset();

          /** \brief Returns the adequate string for the templated time.
           *
           */
          QString printElapsed() const;

        private:
          /** \brief Returns the adequate string for the templated time units.
           *
           */
          QString printUnits() const;

          /** \brief Returns "X hours, Y minutes, Z seconds" formated time units.
           *
           * \param[in] time current time.
           */
          QString prettyPrint(std::chrono::high_resolution_clock::time_point time) const;

        private:
          const QString m_id; /** Timer identifier */
          std::chrono::high_resolution_clock::time_point m_startTime; /** Timer creation time. */
      };

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline BlockTimer<TimeUnits>::BlockTimer(const QString &name)
          : m_id { name }, m_startTime { std::chrono::high_resolution_clock::now() }
      {
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline BlockTimer<TimeUnits>::~BlockTimer()
      {
        qDebug() << printElapsed();
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline int BlockTimer<TimeUnits>::elapsed() const
      {
        auto nowTime = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<TimeUnits>(nowTime - m_startTime).count();
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline void BlockTimer<TimeUnits>::reset()
      {
        m_startTime = std::chrono::high_resolution_clock::now();
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline QString BlockTimer<TimeUnits>::printElapsed() const
      {
        auto endTime = std::chrono::high_resolution_clock::now();

        QString message = m_id + " execution time: ";
        if (std::is_same<TimeUnits, std::chrono::milliseconds>::value)
        {
          message += prettyPrint(endTime);
        }
        else
        {
          auto number = std::chrono::duration_cast<TimeUnits>(endTime - m_startTime).count();
          message += QString::number(number) + ' ' + printUnits();
        }

        return message;
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline QString BlockTimer<TimeUnits>::printUnits() const
      {
        if (std::is_same<typename TimeUnits::period, std::chrono::nanoseconds::period>::value) return "nanoseconds";

        if (std::is_same<typename TimeUnits::period, std::chrono::microseconds::period>::value) return "microseconds";

        if (std::is_same<typename TimeUnits::period, std::chrono::seconds::period>::value) return "seconds";

        if (std::is_same<typename TimeUnits::period, std::chrono::minutes::period>::value) return "minutes";

        if (std::is_same<typename TimeUnits::period, std::chrono::hours::period>::value) return "hours";

        return "unknown units";
      }

      //------------------------------------------------------------------------
      template<typename TimeUnits>
      inline QString BlockTimer<TimeUnits>::prettyPrint(std::chrono::high_resolution_clock::time_point time) const
      {
        auto timeMilis = std::chrono::duration_cast<std::chrono::milliseconds>(time - m_startTime).count();
        auto timeSeconds = timeMilis/1000;

        auto hours = timeSeconds / 3600;
        auto minutes = timeSeconds % 3600 / 60;
        auto seconds = timeSeconds % 60; //= timeSeconds % 3600 % 60
        auto milliseconds = timeMilis % 1000;

        QString message;
        message.append(hours ? (QString::number(hours) + " hours, ") : "");
        message.append(minutes ? (QString::number(minutes) + " minutes, ") : "");
        message.append(seconds ? (QString::number(seconds) + " seconds, ") : "");
        message.append((message.isEmpty() || milliseconds) ? QString::number(milliseconds) + " milliseconds" : "");
        if (message.endsWith(", ")) message.chop(2);

        return message;
      }

    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

#endif // CORE_UTILS_BLOCKTIMER_HXX_
