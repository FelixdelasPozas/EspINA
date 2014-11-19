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


namespace ESPINA {
  namespace Testing {
    class DummyFilter
    : public Filter
    {
    public:
      explicit DummyFilter()
      : Filter(InputSList(), "Dummy", SchedulerSPtr(new Scheduler(10000000)))
      {}
      virtual void restoreState(const State& state) {}
      virtual State state() const{ return State();}

    protected:
      virtual Snapshot saveFilterSnapshot() const { return Snapshot(); }
      virtual bool needUpdate() const {return true;}
      virtual bool needUpdate(Output::Id id) const {return true;}
      virtual void execute(){ execute(0); }
      virtual void execute(Output::Id id)
      {
        if (!m_outputs.contains(0)) {
          m_outputs[0] = OutputSPtr(new Output(this, 0, NmVector3{1,1,1}));
        }

        Bounds bounds{0,5,0,5,0,5};
        DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(bounds)};

        // Modify output data during filter execution
        // it should be cleared by filter update after execution
        itkVolumeType::IndexType index;
        index.Fill(0);
        volume->draw(index);

        m_outputs[0]->setData(volume);
      }
      virtual bool ignoreStorageContent() const {return true;}
      virtual bool areEditedRegionsInvalidated(){return false;}
    };
  }
}


#endif // TESTING_DUMMYFILTER_H
