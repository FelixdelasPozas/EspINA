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

#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Query.h>

#include "query_testing_support.h"

using namespace std;
using namespace EspINA;
using namespace EspINA::Testing;

int query_segmentation_channel(int argc, char** argv )
{
  bool error = false;

  Analysis analysis;

  SampleSPtr sample(new Sample());
  analysis.add(sample);

  FilterSPtr filter{new DummyFilter()};

  ChannelSPtr channel(new Channel(filter, 0));

  OutputSList inputs;
  inputs << filter->output(0);

  FilterSPtr filterWithInputs{new DummyFilterWithInputs(inputs)};

  SegmentationSPtr segmentation(new Segmentation(filterWithInputs, 0));

  analysis.add(channel);
  analysis.add(segmentation);

  auto segChannels = Query::channels(segmentation);

  if (segChannels.size() != 1) {
    cerr << "Unexpected number of segmentation channels. Expected only 1 channel" << endl;
    error = true;
  }

  if (segChannels.first() != channel) {
    cerr << "Unexpected segmentation channel" << endl;
    error = true;
  }

  return error;
}