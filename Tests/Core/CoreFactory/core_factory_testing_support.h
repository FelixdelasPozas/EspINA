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

#ifndef TESTING_DUMMYFILTER_H
#define TESTING_DUMMYFILTER_H

#include <Core/Factory/FilterFactory.h>

namespace ESPINA
{
  namespace Testing
  {
    class DummyFilter
    : public Filter
    {
    public:
      explicit DummyFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler)
      : Filter(inputs, type, scheduler){}

      virtual OutputSPtr output(Output::Id id) const {return OutputSPtr();}
      virtual void restoreState(const State& state){}
      virtual State state() const{return State();}

      void dummyMethod(){}

    protected:
      virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }
      virtual bool needUpdate() const { return false; }
      virtual bool needUpdate(Output::Id id) const{ return false; }
      virtual void execute(){}
      virtual void execute(Output::Id id){}
      virtual bool ignoreStorageContent() const {return false;}
      virtual bool areEditedRegionsInvalidated() {return false;}
    };

    class DummyFilterFactory
    : public FilterFactory
    {
    public:
      static Filter::Type dummyType() { return "DummyFilter"; }
      virtual const FilterTypeList providedFilters() const
      { FilterTypeList list; list << dummyType(); return list; }

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
      { return std::make_shared<DummyFilter>(inputs, filter, scheduler);}
    };
// 
//     class DummyProvider
//     : public ExtensionProvider
//     {
//     public:
//       virtual void restoreState(const State& state) {}
//       virtual void saveState(State& state) const{}
//       virtual Snapshot saveSnapshot() const {return Snapshot();}
//       virtual void unload() {}
//       virtual Type type() const {}
//       virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type& type) {}
//       virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type& type) {}
//     };
  }
}

#endif // TESTING_DUMMYFILTER_H
