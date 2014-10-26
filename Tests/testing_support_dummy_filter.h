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

#ifndef TESTING_SUPPORT_DUMMY_FILTER_H
#define TESTING_SUPPORT_DUMMY_FILTER_H

#include <Core/Analysis/Filter.h>
#include <Core/Analysis/DataProxy.h>
#include <Core/MultiTasking/Scheduler.h>

namespace ESPINA {
  namespace Testing {
    class DummyFilter
    : public Filter
    {
    public:
      explicit DummyFilter();

      virtual void restoreState(const State& state) override {}
      virtual State state() const                   override {return State();}

    protected:
    virtual Snapshot saveFilterSnapshot() const     override {return Snapshot(); }
      virtual bool needUpdate() const               override {return false;}
      virtual bool needUpdate(Output::Id id) const  override {return false;}
      virtual void execute()                        override {}
      virtual void execute(Output::Id id)           override {}
      virtual bool ignoreStorageContent() const     override {return false;}
    };
  }

  class DummyData 
  : public Data
  {
  public:
    virtual Type type() const {return "DummyData";}
    virtual bool isValid() const {return true;}
    virtual bool isEmpty() const {return false;}
    virtual Bounds bounds() const {}
    virtual void setSpacing(const NmVector3& spacing){}
    virtual NmVector3 spacing() const {return NmVector3({1,1,1});}
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString& path, const QString& id) { return false; }
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const {return Snapshot();}
    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const { return Snapshot();}
    virtual DataProxySPtr createProxy() const;
    virtual size_t memoryUsage() const {return 0;}
    virtual void undo() {};
  };

  using DummyDataSPtr = std::shared_ptr<DummyData>;

  class DummyDataProxy
  : public DataProxy
  {
  public:
    virtual DataSPtr get() const
    { return m_data; }
    virtual void set(DataSPtr data)
    { m_data = std::dynamic_pointer_cast<DummyData>(data); }

  private:
    DummyDataSPtr m_data;
  };
}

#endif // TESTING_SUPPORT_DUMMY_FILTER_H
