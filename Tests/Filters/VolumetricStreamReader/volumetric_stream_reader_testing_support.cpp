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
#include "volumetric_stream_reader_testing_support.h"

#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>

using namespace ESPINA;
using namespace ESPINA::Testing;

using ChannelVolume = SparseVolume<itkVolumeType>;

OutputSPtr ESPINA::Testing::inputChannel()
{
  class DummyFilter
  : public Filter
  {
  public:
    explicit DummyFilter()
    : Filter(OutputSList(), "Dummy", SchedulerSPtr()){}
    virtual OutputSPtr output(Output::Id id) const {}

  protected:
    virtual Snapshot saveFilterSnapshot() const {return Snapshot(); }
    virtual bool needUpdate() const{ return false;}
    virtual bool needUpdate(Output::Id id) const{ return false;}
    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type){}
    virtual void execute(){}
    virtual void execute(Output::Id id){}
    virtual bool invalidateEditedRegions() {return false;}
  };

  OutputSPtr output{new Output(new DummyFilter(),0)};

  DataSPtr data{new ChannelVolume()};

  output->setData(data);

  return output;
}
