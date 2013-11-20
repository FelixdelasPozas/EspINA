/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "Core/Analysis/Output.h"
#include <Core/Analysis/Filter.h>
#include <Core/MultiTasking/Scheduler.h>

using namespace EspINA;
using namespace std;


int output_update_filter( int argc, char** argv )
{
  class DummyFilter
  : public Filter 
  {
  public:
    explicit DummyFilter()
    : Filter(OutputSList(), "Dummy", SchedulerSPtr(new Scheduler(10000000)))
    , UpdatedOutput{-1}{}
    virtual void restoreState(const State& state) {}
    virtual State state() const{ return State();}
    
    int UpdatedOutput;
    
  protected:
    virtual Snapshot saveFilterSnapshot() const {}
    virtual bool needUpdate() const {return true;}
    virtual bool needUpdate(Output::Id id) const {return true;}
    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type){}
    virtual void execute(){}
    virtual void execute(Output::Id id){UpdatedOutput = id;}
    virtual bool ignoreStorageContent() const {return false;}
    virtual bool invalidateEditedRegions() {return false;}
  };
  
  bool error = false;

  DummyFilter *filter{new DummyFilter()};

  Output::Id id = 0;
  
  Output output(filter, id);
  
  output.update();
  
  if (filter->UpdatedOutput != id) {
    cerr << "Unexpected filter update for output " << id << endl;
    error = true;
  }
  
  id = 5;
  Output output2(filter, id);
  
  output2.update();
  
  if (filter->UpdatedOutput != id) {
    cerr << "Unexpected filter update for output " << id << endl;
    error = true;
  }
  
  return error;
}