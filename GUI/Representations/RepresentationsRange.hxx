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

#ifndef ESPINA_REPRESENTATIONS_RANGE_H
#define ESPINA_REPRESENTATIONS_RANGE_H

#include <Core/EspinaTypes.h>
#include <QMap>

namespace ESPINA
{
  template<typename R>
  class RepresentationsRange
  {
  public:
    RepresentationsRange();

    TimeRange timeRange() const;

    R last() const;

    TimeStamp lastTime() const;

    void addValue(R representation, TimeStamp t);

    void reusePreviousValue(TimeStamp t);

    R value(TimeStamp t, R invalid) const;

    void invalidate();

    void invalidatePreviousValues(TimeStamp t);

    bool isEmpty() const;

  private:
    TimeRange m_times;
    TimeStamp m_lastTime;
    QMap<TimeStamp, R> m_representations;
  };

#include "RepresentationsRange.cxx"
}

#endif // ESPINA_REPRESENTATIONS_RANGE_H
