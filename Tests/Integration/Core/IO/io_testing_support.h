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
#include <Core/Analysis/Extensions/ExtensionProvider.h>
#include <Core/MultiTasking/Scheduler.h>

namespace EspINA {
  namespace IO_Testing {
    class DummyFilter
    : public Filter
    {
    public:
      explicit DummyFilter()
      : Filter(OutputSList(), "DummyFilter", SchedulerSPtr(new Scheduler(10000000)))
      { setName("DummyFilter");
        m_output = OutputSPtr{new Output(this, 0)};
      }
      virtual void restoreState(const State& state) {}
      virtual State saveState() const {return State();}
      virtual OutputSPtr output(Output::Id id) const {return m_output;}

    protected:
      virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }
      virtual bool needUpdate() const {return false;}
      virtual bool needUpdate(Output::Id id) const {return false;}
      virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type) {return DataSPtr();}
      virtual void execute(){}
      virtual void execute(Output::Id id){}
      virtual bool invalidateEditedRegions() {return false;}
      
      OutputSPtr m_output;
    };

    class DummyProvider 
    : public ExtensionProvider
    {
    public:
      virtual void restoreState(const State& state) {}
      virtual State saveState() const {return State();}
      virtual Snapshot saveSnapshot() const {return Snapshot();}
      virtual void unload() {}
      virtual Type type() const { return Type();}
      virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type& type) { return ChannelExtensionSPtr();}
      virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type& type) {return SegmentationExtensionSPtr();}
    };
  }
}

#endif // TESTING_DUMMYFILTER_H
