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

// ESPINA
#include "Timer.h"

// C++
#include <limits>

using namespace ESPINA;

const TimeStamp Timer::INVALID_TIME_STAMP = 0;
const TimeStamp Timer::INITIAL_TIME_STAMP = 1;
const TimeStamp Timer::MAXIMUM_TIME_STAMP = std::numeric_limits<std::uint64_t>::max();

//------------------------------------------------------------------------
Timer::Timer(TimeStamp time)
: m_timeStamp{time}
{
}

//------------------------------------------------------------------------
TimeStamp Timer::increment()
{
  QMutexLocker lock(&m_mutex);
  if(willOverflow())
  {
    resetImplemenation();
  }
  else
  {
    ++m_timeStamp;

    emit tic(m_timeStamp);
  }

  return m_timeStamp;
}

//------------------------------------------------------------------------
TimeStamp Timer::reset()
{
  QMutexLocker lock(&m_mutex);

  resetImplemenation();

  return m_timeStamp;
}

//------------------------------------------------------------------------
bool Timer::isValid(TimeStamp t)
{
  return t != INVALID_TIME_STAMP;
}

//------------------------------------------------------------------------
void Timer::resetImplemenation()
{
  m_timeStamp = INITIAL_TIME_STAMP;

  emit reset(m_timeStamp);
}

//------------------------------------------------------------------------
bool Timer::willOverflow() const
{
  return m_timeStamp == MAXIMUM_TIME_STAMP;
}
