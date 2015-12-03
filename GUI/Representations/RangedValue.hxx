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
#include <GUI/Utils/Timer.h>

// Qt
#include <QMap>
#include <QDebug>
#include <QReadWriteLock>

namespace ESPINA
{
  template<typename R> class RangedValue;

  /** \brief Prints all the values in the range, for debugging purposes.
   *
   * TODO: doesn't work if T is a smart pointer.
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

      TimeStamp          m_lastTime; /** last range time that the object has a value for. */
      QMap<TimeStamp, R> m_values;   /** time-value map. */
      R                  m_default;  /** default value to return. Needed to avoid returning a temporary in case of smartpointers. */

      mutable QReadWriteLock m_mutex;
  };

  //-----------------------------------------------------------------------------
  template<typename R>
  RangedValue<R>::RangedValue()
  : m_lastTime{Timer::INVALID_TIME_STAMP}
  {
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  TimeRange RangedValue<R>::timeRange() const
  {
    QReadLocker lock(&m_mutex);

    TimeRange range;

    if (!m_values.isEmpty())
    {
      for (auto i = m_values.keys().first(); i <= m_lastTime; ++i)
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
    QReadLocker lock(&m_mutex);

    auto result = m_default;

    if (!m_values.isEmpty())
    {
      result = m_values.value(m_values.keys().last());
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  TimeStamp RangedValue<R>::lastTime() const
  {
    QReadLocker lock(&m_mutex);

    return m_lastTime;
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::addValue(R value, TimeStamp t)
  {
    QWriteLocker lock(&m_mutex);

    if (m_lastTime < t || m_values.isEmpty())
    {
      m_lastTime = t;
    }

    m_values.insert(t, value);
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::reusePreviousValue(TimeStamp t)
  {
    QWriteLocker lock(&m_mutex);

    if(m_lastTime < t)
    {
      m_lastTime = t;
    }
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  R RangedValue<R>::value(TimeStamp t, R invalid) const
  {
    QReadLocker lock(&m_mutex);

    if(m_values.empty() || (t > m_lastTime) || (t < m_values.keys().first())) return invalid;

    auto time = t;
    while((m_values.find(time) == m_values.end()) && (time >= m_values.keys().first()))
    {
      --time;
    }

    Q_ASSERT(m_values.find(time) != m_values.end());

    return m_values.value(time, invalid);
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::invalidate()
  {
    QWriteLocker lock(&m_mutex);

    m_lastTime = 0;
    m_values.clear();
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  void RangedValue<R>::invalidatePreviousValues(TimeStamp t)
  {
    QWriteLocker lock(&m_mutex);

    if (m_values.isEmpty() || t < m_values.keys().first()) return;
    Q_ASSERT(t <= m_lastTime);

    if(m_values.find(t) == m_values.end())
    {
      auto time = t;
      while(m_values.find(time) == m_values.end())
      {
        --time;
      }
      Q_ASSERT(m_values.find(time) != m_values.end());

      auto timeValue = m_values.value(time);
      m_values.insert(t, timeValue);
    }

    for(auto time: m_values.keys())
    {
      if(time < t)
      {
        m_values.remove(time);
      }
      else
      {
        return;
      }
    }

    if(m_values.empty())
    {
      m_lastTime = 0;
    }
  }

  //-----------------------------------------------------------------------------
  template<typename R>
  bool RangedValue<R>::isEmpty() const
  {
    QReadLocker lock(&m_mutex);

    return m_values.isEmpty();
  }

  //-----------------------------------------------------------------------------
  template<typename T> QDebug operator<<(QDebug debug, RangedValue<T> range)
  {
    debug << "\n---RangedValue---------------\n";
    debug << "last:" << range.m_lastTime << "\n";
    for (auto key : range.m_values.keys())
    {
      debug << key << range.m_values.value(key);
    }
    debug << "\n-----------------------------\n";

    return debug;
  }
}

#endif // ESPINA_RANGED_VALUE_H
