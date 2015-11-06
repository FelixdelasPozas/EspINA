/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ESPINA_RANGED_VALUE_H
#define ESPINA_RANGED_VALUE_H

// ESPINA
#include <Core/Types.h>

// Qt
#include <QMap>
#include <QDebug>

namespace ESPINA
{
  template<typename R> class RangedValue;

  /** \brief Prints all the values in the range, for debugging purposes.
   *
   */
  template<typename T> QDebug operator<<(QDebug debug, RangedValue<T> range);

  template<typename R>
  class RangedValue
  {
    public:
      /** \brief RangedValue class constructor.
       *
       */
      explicit RangedValue();

      /** \brief Returns the group of timestamps the class has values for.
       *
       */
      TimeRange timeRange() const;

      /** \brief Returns the value for the biggest timestamp.
       *
       */
      R last() const;

      /** \brief Returns the biggest timestamp.
       *
       */
      TimeStamp lastTime() const;

      /** \brief Adds a value associated to a timestamp.
       * \param[in] value
       * \param[in] t timestamp value.
       *
       */
      void addValue(R value, TimeStamp t);

      /** \brief Reuses the value asociated to the last timestamp to the new timestamp.
       * \param[in] t timestamp value.
       *
       */
      void reusePreviousValue(TimeStamp t);

      /** \brief Returns the value asociated to the specified timestamp.
       * \param[in] t timestamp value.
       * \param[in] invalid invalid value to return in case there is no value associated to the timestamp.
       *
       */
      R value(TimeStamp t, R invalid = R()) const;

      /** \brief Empties the range.
       *
       */
      void invalidate();

      /** \brief Removes the values previous to the given timestamp.
       * \param[in] t timestamp value.
       *
       */
      void invalidatePreviousValues(TimeStamp t);

      /** \brief Returns true if the range is empty (has no values).
       *
       */
      bool isEmpty() const;

    private:
      friend QDebug operator<< <>(QDebug debug, RangedValue range);

      TimeRange m_times;
      TimeStamp m_lastTime;
      QMap<TimeStamp, R> m_values;
  };

  //-----------------------------------------------------------------------------
  template<typename R>
  RangedValue<R>::RangedValue()
  : m_lastTime{0}
  {
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  TimeRange RangedValue<R>::timeRange() const
  {
    TimeRange range;

    if (!m_times.isEmpty())
    {
      for (auto i = m_times.first(); i <= m_lastTime; ++i)
      {
        range << i;
      }
    }

    return range;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  R RangedValue<R>::last() const
  {
    R result;

    if (!m_times.isEmpty())
    {
      Q_ASSERT(m_values.contains(m_times.last()));

      result = m_values[m_times.last()];
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  TimeStamp RangedValue<R>::lastTime() const
  {
    return m_lastTime;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::addValue(R value, TimeStamp t)
  {
    if (m_lastTime < t || m_values.isEmpty())
    {
      m_lastTime = t;
    }

    auto needSort = !m_times.isEmpty() && t < m_times.last();

    m_times << t;

    if (needSort)
    {
      qSort(m_times);
    }

    m_values[t] = value;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::reusePreviousValue(TimeStamp t)
  {
    if(!m_times.empty() && m_lastTime <= t) m_lastTime = t;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  R RangedValue<R>::value(TimeStamp t, R invalid) const
  {
    TimeStamp valueTime = 0;

    if (m_times.isEmpty())
    {
      qWarning() << "Accessing invalid value";
    }
    else
    {
      valueTime = m_times.first();
    }

    int i = 1;
    bool found = false;

    while (!found && i < m_times.size())
    {
      auto nextTime = m_times[i];

      found = nextTime > t;

      if (!found)
      {
        valueTime = nextTime;
      }

      ++i;
    }

    return m_values.value(valueTime, invalid);
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::invalidate()
  {
    m_lastTime = 0;
    m_times.clear();
    m_values.clear();
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::invalidatePreviousValues(TimeStamp t)
  {
    if (m_times.isEmpty())
      return;

    bool found = false;
    auto validTime = m_times.takeFirst();
    auto validRepresentations = m_values[validTime];

    while (!found && !m_times.isEmpty())
    {
      auto nextTime = m_times.takeFirst();

      found = nextTime > t;

      if (!found)
      {
        m_values.remove(validTime);

        validTime = nextTime;
        validRepresentations = m_values[nextTime];
      }
    }

    m_times.prepend(t);
    m_values[t] = validRepresentations;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  bool RangedValue<R>::isEmpty() const
  {
    return m_values.isEmpty();
  }

  //-----------------------------------------------------------------------------
  template<typename T> QDebug operator<<(QDebug debug, RangedValue<T> range)
  {
    debug << "\n---RangedValue---------------\n";
    debug << "last:" << range.m_lastTime << "\n";
    debug << "times:" << range.m_times << "\n";
    for (auto key : range.m_values.keys())
    {
      debug << key << range.m_values[key];
    }
    debug << "\n-----------------------------\n";

    return debug;
  }
}

#endif // ESPINA_RANGED_VALUE_H
