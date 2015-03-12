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

// ESPINA
#include "Timer.h"

// C++
#include <limits>

namespace ESPINA
{
  //------------------------------------------------------------------------
  Timer::Timer(TimeStamp time)
  : QObject()
  , m_timeStamp  {time}
  , m_initialTime{time}
  {
  }

  //------------------------------------------------------------------------
  TimeStamp Timer::increment()
  {
    ++m_timeStamp;

    if(m_timeStamp == std::numeric_limits<std::uint64_t>::max())
    {
      reset();
    }
    else
    {
      emit tic(m_timeStamp);
    }

    return m_timeStamp;
  }

  //------------------------------------------------------------------------
  TimeStamp Timer::reset()
  {
    m_timeStamp = m_initialTime;
    emit reset(m_timeStamp);

    return m_timeStamp;
  }

} /* namespace ESPINA */