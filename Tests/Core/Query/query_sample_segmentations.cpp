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
using namespace ESPINA;
using namespace ESPINA::Testing;

int query_sample_segmentations(int argc, char** argv )
{
  bool error = false;

  Analysis analysis;

  SampleSPtr sample(new Sample());
  analysis.add(sample);

  FilterSPtr filter{new DummyFilter()};
  auto input = getInput(filter, 0);

  ChannelSPtr channel(new Channel(input));

  InputSList inputs;
  inputs << input;

  FilterSPtr filterWithInputs{new DummyFilterWithInputs(inputs)};
  auto input1 = getInput(filterWithInputs, 0);

  SegmentationSPtr segmentation(new Segmentation(input1));

  analysis.add(channel);
  analysis.add(segmentation);

  auto sampleSegmentations = QueryRelations::segmentations(sample);

  if (!sampleSegmentations.isEmpty()) {
    cerr << "Unexpected sample's segmentations" << endl;
    error = true;
  }

  analysis.addRelation(sample, segmentation, Sample::CONTAINS);

  sampleSegmentations = QueryRelations::segmentations(sample);

  if (sampleSegmentations.size() != 1) {
    cerr << "Unexpected number of sample segmentations. Expected only 1 segmentation" << endl;
    error = true;
  }


  if (sampleSegmentations.first() != segmentation) {
    cerr << "Unexpected sample segmentation" << endl;
    error = true;
  }

  return error;
}
