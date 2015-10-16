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

#include <Core/Types.h>
#include <QMap>
#include <QDebug>

namespace ESPINA
{
  template<typename R>
  class RangedValue
  {
  public:
    RangedValue();

    TimeRange timeRange() const;

    R last() const;

    TimeStamp lastTime() const;

    void addValue(R value, TimeStamp t);

    void reusePreviousValue(TimeStamp t);

    R value(TimeStamp t, R invalid = R()) const;

    void invalidate();

    void invalidatePreviousValues(TimeStamp t);

    bool isEmpty() const;

    void print() const
    {
      for (auto key : m_values.keys())
      {
        qDebug() << "\t" << key << m_values[key];
      }
    }

//     QDebug operator<<(QDebug &debug) const
//     {
//       debug << QString("[%1@%2]").arg(last()).arg(lastTime()) << m_values.keys();
//       return debug;
//     }

  private:
    TimeRange m_times;
    TimeStamp m_lastTime;
    QMap<TimeStamp, R> m_values;
  };
}

#include "RangedValue.cxx"

#endif // ESPINA_RANGED_VALUE_H
