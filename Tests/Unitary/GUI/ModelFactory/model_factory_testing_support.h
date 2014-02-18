/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef TESTING_DUMMYFILTER_H
#define TESTING_DUMMYFILTER_H

#include <Core/Analysis/Filter.h>
#include <Core/MultiTasking/Scheduler.h>

namespace EspINA {
  namespace Testing {
    class DummyFilter
    : public Filter
    {
    public:
      explicit DummyFilter(InputSList inputs, Filter::Type& type, SchedulerSPtr scheduler)
      : Filter(inputs, type, scheduler)
      { m_outputs[0] = OutputSPtr(new Output(this, 0)); }

      virtual void restoreState(const State& state) {}
      virtual State state() const {return State();}

      void dummyMethod(){}
    protected:
      virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }
      virtual bool needUpdate() const {return false;}
      virtual bool needUpdate(Output::Id id) const{return false;}
      virtual void execute(){}
      virtual void execute(Output::Id id){}
      virtual bool ignoreStorageContent() const {return false;}
      virtual bool invalidateEditedRegions() {return false;}
    };
  }
}

#endif // TESTING_DUMMYFILTER_H
