/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "testing_support_dummy_filter.h"

using namespace ESPINA;
using namespace ESPINA::Testing;

//------------------------------------------------------------------------
DummyFilter::DummyFilter()
: Filter(InputSList(), "DummyFilter", SchedulerSPtr(new Scheduler(10000000)))
{
  m_outputs[0] = std::make_shared<Output>(this, 0, NmVector3{1,1,1});
}

//------------------------------------------------------------------------
DummyFilter::DummyFilter(InputSList input, Filter::Type type, SchedulerSPtr scheduler)
: Filter(input, type, scheduler)
{
  m_outputs[0] = std::make_shared<Output>(this, 0, NmVector3{1,1,1});
}

//------------------------------------------------------------------------
DataSPtr DummyData::createProxy() const
{
  return std::make_shared<DummyDataProxy>();
}
