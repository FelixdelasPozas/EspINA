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
#include "seed_grow_segmentation_testing_support.h"

#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>

using namespace EspINA;
using namespace EspINA::Testing;

using ChannelVolume = SparseVolume<itkVolumeType>;

OutputSPtr EspINA::Testing::inputChannel()
{
  class DummyFilter
  : public Filter
  {
  public:
    explicit DummyFilter()
    : Filter(OutputSList(), "Dummy", SchedulerSPtr()){}
    virtual void restoreState(const State& state){}
    virtual State saveState() const{return State();}
  protected:
    virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }
    virtual bool needUpdate() const{ return false;}
    virtual bool needUpdate(Output::Id id) const{ return false;}
    virtual void execute(){}
    virtual void execute(Output::Id id){}
    virtual bool invalidateEditedRegions() {return false;}
  };

  OutputSPtr output{new Output(new DummyFilter(),0)};

  Bounds bounds{-0.5, 99.5, -0.5,99.5,-0.5,99.5};

  DefaultVolumetricDataSPtr data{new ChannelVolume(bounds)};
  data->setBackgroundValue(50);

  output->setData(data);

  return output;
}
