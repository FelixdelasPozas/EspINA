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

namespace ESPINA
{
  //------------------------------------------------------------------------
  Timer::Timer(TimeStamp time)
  : QObject()
  , m_timeStamp{time}
  {
  }

  //------------------------------------------------------------------------
  TimeStamp Timer::increment()
  {
    ++m_timeStamp;

    // TODO: manage overflow and emit resetTimer(TimeStamp) when needed.
    //       we must avoid using 0 because it is used as invalidation value on Representation Managers/Pools

    emit tic(m_timeStamp);

    return m_timeStamp;
  }

} /* namespace ESPINA */
