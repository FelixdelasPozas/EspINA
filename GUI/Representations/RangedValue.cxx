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

#include <GUI/Representations/RangedValue.hxx>

namespace ESPINA
{
  template<typename R>
  RangedValue<R>::RangedValue()
  : m_lastTime{0}
  {
  }

  template<typename R>
  TimeRange RangedValue<R>::timeRange() const
  {
    TimeRange range;

    if (!m_times.isEmpty())
    {
      for (TimeStamp i = m_times.first(); i <= m_lastTime; ++i)
      {
        range << i;
      }
    }

    return range;
  }

  template<typename R>
  R RangedValue<R>::last() const
  {
    R result;

    if (!m_times.isEmpty())
    {
      Q_ASSERT(m_representations.contains(m_times.last()));
      result = m_representations[m_times.last()];
    }

    return result;
  }

  template<typename R>
  TimeStamp RangedValue<R>::lastTime() const
  {
    return m_lastTime;
  }

  template<typename R>
  void RangedValue<R>::addValue(R representation, TimeStamp t)
  {
    if(m_lastTime < t)
    {
      m_lastTime = t;
    }
    else
    {
      qWarning() << "Adding previous value";
    }

    auto needSort = !m_times.isEmpty() && t < m_times.last();

    m_times << t;

    if (needSort)
    {
      qWarning() << "Sorting unordered time stamp";
      qSort(m_times);
    }

    m_representations[t] = representation;
  }

  template<typename R>
  void RangedValue<R>::reusePreviousValue(TimeStamp t)
  {
    Q_ASSERT(!m_times.isEmpty() && m_lastTime <= t);
    m_lastTime = t;
  }

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

    int  i     = 1;
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

    return m_representations.value(valueTime, invalid);
  }


  template<typename R>
  void RangedValue<R>::invalidate()
  {
    m_lastTime = 0;
    m_times.clear();
    m_representations.clear();
  }

  template<typename R>
  void RangedValue<R>::invalidatePreviousValues(TimeStamp t)
  {
    if (m_times.isEmpty()) return;

    bool found                = false;
    auto validTime            = m_times.takeFirst();
    auto validRepresentations = m_representations[validTime];

    while (!found && !m_times.isEmpty())
    {
      auto nextTime = m_times.takeFirst();

      found = nextTime > t;

      if (!found)
      {
        m_representations.remove(validTime);

        validTime            = nextTime;
        validRepresentations = m_representations[nextTime];
      }
    }

    m_times.prepend(t);
    m_representations[t] = validRepresentations;
  }

  template<typename R>
  bool RangedValue<R>::isEmpty() const
  {
    return m_representations.isEmpty();
  }
}