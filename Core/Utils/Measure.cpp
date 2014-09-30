/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "Core/Utils/Measure.h"

namespace ESPINA
{
	//-----------------------------------------------------------------------------
  void Measure::adjust()
  {
    if (!m_adjust)
    {
      if (m_units == QString("Nm") || m_units == QString("nm"))
        return;

      if (m_units == QString("Mm") || m_units == QString("mm"))
        m_measure *= 1000000;

      if (m_units == QString("Um") || m_units == QString("um"))
        m_measure *= 1000;

      m_units = QString("nm");
      return;
    }

    while((m_measure / 1000.0) >= 1)
    {
      if (m_units == QString("Mm") || m_units == QString("mm"))
        return;

      m_measure /= 1000.0;
      if (m_units == QString("Nm") || m_units == QString("nm"))
          m_units = "um";
      else
      {
        if (m_units == QString("Um") || m_units == QString("um"))
          m_units = "mm";
        else
          Q_ASSERT(false);
      }
    }
  }

  //-----------------------------------------------------------------------------
  void Measure::toUnits(QString units)
  {
    if (!m_unitsStrings.contains(units) || units == m_units)
      return;

    if (m_units == QString("nm"))
    {
      if (units == QString("um") || units == QString("Um"))
        m_measure /= 1000;
      else
        m_measure /= 1000000;
    }

    if (m_units == QString("um"))
    {
      if (units == QString("nm") || units == QString("Nm"))
        m_measure *= 1000;
      else
        m_measure /= 1000;
    }

    if (m_units == QString("mm"))
    {
      if (units == QString("nm") || units == QString("Nm"))
        m_measure *= 1000000;
      else
        m_measure *= 1000;
    }

    m_units = units;
    m_adjust = false;
  }
} /* namespace ESPINA */

