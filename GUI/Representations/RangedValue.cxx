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
  m_lastTime = t;
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
  int  i     = 1;
  bool found = false;

  Q_ASSERT(!m_times.isEmpty());

  TimeStamp validTime = m_times.first();

  while (!found && i < m_times.size())
  {
    auto nextTime = m_times[i];

    found = nextTime > t;

    if (!found)
    {
      validTime = nextTime;
    }

    ++i;
  }

  return m_representations.value(validTime, invalid);
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
  int  i     = 1;
  bool found = false;

  if (m_times.isEmpty()) return;

  auto validTime            = m_times.first();
  auto validRepresentations = m_representations[validTime];

  while (!found && i < m_times.size())
  {
    auto nextTime = m_times[i];

    found = nextTime > t;

    if (!found)
    {
      m_representations.remove(validTime);

      validTime            = nextTime;
      validRepresentations = m_representations[nextTime];

      ++i;
    }
  }

  while (i > 1) // remove invalid timestamps except one to be replaced
  {
    m_times.removeFirst();
    --i;
  }

  m_times[0]      = t;
  m_representations[t] = validRepresentations;
}

template<typename R>
bool RangedValue<R>::isEmpty() const
{
  return m_representations.isEmpty();
}
