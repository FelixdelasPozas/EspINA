/*
 * Measure.cpp
 *
 *  Created on: Jan 11, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "Core/Utils/Measure.h"

namespace EspINA
{
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
} /* namespace EspINA */

