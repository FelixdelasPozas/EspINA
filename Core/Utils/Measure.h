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

#ifndef ESPINA_MEASURE_H_
#define ESPINA_MEASURE_H_

#include "Core/EspinaCore_Export.h"

// Qt
#include <QStringList>
#include <QString>

// C++
#include <memory>

namespace ESPINA
{
  class EspinaCore_EXPORT Measure
  {
    public:
  		/* \brief Measure class constructor.
  		 *
  		 */
      Measure()
      : m_measure(0)
      , m_units("nm")
      , m_adjust(true)
      {
      	m_unitsStrings << "Mm" << "Um" << "Nm" << "mm" << "um" << "nm";
      };

  		/* \brief Measure class constructor.
  		 * \param[in] measure, numerical value.
  		 * \param[in] units, string of measure units.
  		 *
  		 */
      Measure(double measure, QString units = QString("nm"))
      : m_measure(measure)
      , m_units(units)
      , m_adjust(true)
      {
      	m_unitsStrings << "Mm" << "Um" << "Nm" << "mm" << "um" << "nm";
      	adjust();
      };

  		/* \brief Measure class destructor.
  		 *
  		 */
      virtual ~Measure() {};

  		/* \brief Sets the adjust flag.
  		 *
  		 */
      void SetAdjust(bool enable)
      { m_adjust = enable; adjust(); };

  		/* \brief Returns the adjust flag.
  		 *
  		 */
      bool GetAdjust()
      { return m_adjust; };

  		/* \brief Returns the measure units.
  		 *
  		 */
      QString getUnits()
      { return m_units; };

  		/* \brief Returns the measure.
  		 *
  		 */
      double getMeasure()
      { return m_measure; };

  		/* \brief Sets the units of the measure.
  		 *
  		 */
      void setUnits(QString units)
      { if (m_unitsStrings.contains(units)) m_units = units.toLower(); };

  		/* \brief Sets a new value.
  		 *
  		 */
      void setMeasure(double measure)
      { m_measure = measure; adjust(); };

  		/* \brief Force units conversion.
  		 *
  		 */
      void toUnits(QString units);

    private:
  		/* \brief Helper method for units conversion.
  		 *
  		 */
      void adjust();

      double      m_measure;
      QString     m_units;
      bool        m_adjust;
      QStringList m_unitsStrings;
  };

  using MeasurePtr = Measure *;
  using MeasureSPtr = std::shared_ptr<Measure>;

} /* namespace ESPINA */
#endif // ESPINA_MEASURE_H_
