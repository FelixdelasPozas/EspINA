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

using namespace ESPINA;

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

  m_times << t;
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
  R result = invalid;

  if(!m_times.empty())
  {
    if(m_times.contains(t))
    {
      result = m_representations.value(t, invalid);
    }
    else
    {
      if(m_lastTime <= t && m_times.last() < t)
      {
        result = m_representations.value(m_times.last(), invalid);
      }
      else
      {
        auto i = 0;
        auto time = m_times.first();
        auto previousTime = time;

        while(time < t)
        {
          previousTime = time;
          time = m_times[++i];
        }

        Q_ASSERT(previousTime < t && time > t);
        result = m_representations.value(previousTime, invalid);
      }
    }
  }

  return result;
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
  if(!m_times.empty())
  {
    if(m_times.contains(t))
    {
      while(m_times.first() != t)
      {
        m_representations.remove(m_times.first());
        m_times.removeOne(m_times.first());
      }
    }
    else
    {
      if(m_lastTime <= t && m_times.last() < t)
      {
        auto rep = m_representations[m_times.last()];
        m_times.clear();
        m_representations.clear();

        m_times << t;
        m_representations[t] = rep;
      }
      else
      {
        auto i = 0;
        auto time = m_times.first();
        auto previousTime = time;
        TimeRange rangeToDelete;

        while(time < t)
        {
          previousTime = time;
          rangeToDelete << previousTime;
          time = m_times[++i];
        }

        auto rep = m_representations[previousTime];

        for(auto deletedTime: rangeToDelete)
        {
          m_times.removeOne(deletedTime);
          m_representations.remove(deletedTime);
        }

        m_times.insert(0, t);
        m_representations[t] = rep;
      }
    }
  }
}

template<typename R>
bool RangedValue<R>::isEmpty() const
{
  return m_representations.isEmpty();
}
