/*
    Copyright (C) 2015  Felix de las Pozas Alvarez

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

#ifndef ESPINA_TIMER_H_
#define ESPINA_TIMER_H_

// ESPINA
#include <Core/EspinaTypes.h>

// Qt
#include <QObject>
#include <QMutex>

namespace ESPINA
{
  class Timer
  : public QObject
  {
    Q_OBJECT

    public:
      /** \brief Timer class constructor.
       *
       */
      Timer(TimeStamp time = 0);

      /** \brief Timer class virtual destructor.
       *
       */
      virtual ~Timer()
      {};

      /** \brief Increments the timer and handles the overflow.
       *
       */
      TimeStamp increment();

      /** \brief Returns the current TimeStamp of the timer.
       *
       */
      TimeStamp timeStamp()
      { return m_timeStamp; }

    signals:
      void reset(TimeStamp time);
      void tic(TimeStamp time);

    private:
      TimeStamp m_timeStamp;
      QMutex    m_mutex;
  };

  using TimerPtr  = Timer *;
  using TimerSPtr = std::shared_ptr<Timer>;

} /* namespace ESPINA */

#endif // ESPINA_TIMER_H_
