/*
 * Measure.h
 *
 *  Created on: Jan 11, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef MEASURE_H_
#define MEASURE_H_

#include "Core/EspinaCore_Export.h"

// Qt
#include <QStringList>
#include <QString>

// Boost
#include <boost/shared_ptr.hpp>

namespace EspINA
{
  class EspinaCore_EXPORT Measure
  {
    public:
      Measure(): m_measure(0), m_units("nm"), m_adjust(true)
                 { m_unitsStrings << "Mm" << "Um" << "Nm" << "mm" << "um" << "nm"; };
      Measure(double measure, QString units = QString("nm")): m_measure(measure), m_units(units), m_adjust(true)
                 { m_unitsStrings << "Mm" << "Um" << "Nm" << "mm" << "um" << "nm"; adjust(); };
      virtual ~Measure() {};

      // class configuration getters/setters
      void SetAdjust(bool enable)       { m_adjust = enable; adjust(); };
      bool GetAdjust()                  { return m_adjust; };

      // getters/setters
      QString getUnits()                { return m_units; };
      double getMeasure()               { return m_measure; };
      void setUnits(QString units)      { if (m_unitsStrings.contains(units)) m_units = units.toLower(); };
      void setMeasure(double measure)   { m_measure = measure; adjust(); };

      // force units conversion
      void toUnits(QString units);

    private:
      /// helper methods ///
      // units conversion
      void adjust();

      double      m_measure;
      QString     m_units;
      bool        m_adjust;
      QStringList m_unitsStrings;
  };

  typedef Measure *                  MeasurePtr;
  typedef boost::shared_ptr<Measure> MeasureSPtr;


} /* namespace EspINA */
#endif /* MEASURE_H_ */
