/*
    Copyright (C) 2015  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>

// Qt
#include <QObject>
#include <QMutex>

namespace ESPINA
{
  class EspinaGUI_EXPORT Timer
  : public QObject
  {
      Q_OBJECT

    public:
      static const TimeStamp INVALID_TIME_STAMP; /** value for invalid time. */
      static const TimeStamp INITIAL_TIME_STAMP; /** timer initial value.    */

    public:
      /** \brief Timer class constructor.
       *
       */
      Timer(TimeStamp time = INITIAL_TIME_STAMP);

      /** \brief Timer class virtual destructor.
       *
       */
      virtual ~Timer()
      {};

      /** \brief Returns the current TimeStamp of the timer.
       *
       */
      TimeStamp timeStamp() const
      { return m_timeStamp; }

      /** \brief Resets the timer to the initial TimeStamp.
       *
       */
      TimeStamp reset();

      /** \brief Check if timestamp t is valid or not
       *
       */
      static bool isValid(TimeStamp t);

    public slots:
      /** \brief Increments the timer and handles the overflow.
       *
       */
      TimeStamp increment();

    signals:
      void reset(TimeStamp time);
      void tic(TimeStamp time);

    private:
      /** \brief Resets the timer.
       *
       */
      void resetImplemenation();

      /** \brief Returns true if the next increment of the timer will overflow it.
       *
       */
      bool willOverflow() const;

      static const TimeStamp MAXIMUM_TIME_STAMP;

    private:
      TimeStamp m_timeStamp;
      QMutex    m_mutex;
  };

  using TimerSPtr = std::shared_ptr<Timer>;
} /* namespace ESPINA */

#endif // ESPINA_TIMER_H_
