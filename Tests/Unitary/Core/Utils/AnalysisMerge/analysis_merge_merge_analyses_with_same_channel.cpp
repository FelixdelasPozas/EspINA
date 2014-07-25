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
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Tests/Unitary/Core/Utils/AnalysisMerge/analysis_merge_testing_support.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int analysis_merge_merge_analyses_with_same_channel( int argc, char** argv )
{
  bool error = false;

  AnalysisSPtr analysis1{new Analysis()};

  SampleSPtr sample1{new Sample("Sample")};

  FilterSPtr channelFilter1{new DummyFilter()};
  auto inputC1 = getInput(channelFilter1, 0);

  ChannelSPtr channel1(new Channel(inputC1));
  channel1->setName("Channel 1");

  InputSList inputs1;
  inputs1 << inputC1;

  FilterSPtr segmentationFilter1{new DummyFilter(inputs1)};
  auto inputS1 = getInput(segmentationFilter1, 0);

  SegmentationSPtr segmentation1{new Segmentation(inputS1)};
  segmentation1->setName("Segmentation 1");

  analysis1->add(sample1);
  analysis1->add(channel1);
  analysis1->add(segmentation1);
  analysis1->addRelation(sample1, channel1, Channel::STAIN_LINK);

  AnalysisSPtr analysis2{new Analysis()};

  SampleSPtr sample2{new Sample("Sample")};

  FilterSPtr channelFilter2{new DummyFilter()};
  auto inputC2 = getInput(channelFilter2, 0);

  ChannelSPtr channel2(new Channel(inputC2));
  channel2->setName("Channel 1");

  InputSList inputs2;
  inputs2 << inputC2;

  FilterSPtr segmentationFilter2{new DummyFilter(inputs2)};
  auto inputS2 = getInput(segmentationFilter2, 0);

  SegmentationSPtr segmentation2{new Segmentation(inputS2)};
  segmentation2->setName("Segmentation 2");

  analysis2->add(sample2);
  analysis2->add(channel2);
  analysis2->add(segmentation2);
  analysis2->addRelation(sample2, channel2, Channel::STAIN_LINK);

  AnalysisSPtr merged = merge(analysis1, analysis2);

  if (merged->classification().get() != nullptr) {
    cerr << "Unexpected classification in analysis" << endl;
    error = true;
  }

  if (merged->samples().size() != 1) {
    cerr << "Unexpected number of samples in analysis" << endl;
    error = true;
  }

  if (merged->channels().size() != 1) {
    cerr << "Unexpected number of channels in analysis" << endl;
    error = true;
  }

  if (merged->segmentations().size() != 2) {
    cerr << "Unexpected number of segmentations in analysis" << endl;
    error = true;
  }

  if (merged->relationships()->edges().size() != 1) {
    cerr << "Unexpected number of relationships in analysis" << endl;
    error = true;
  }

  if (analysis1 || analysis2)
  {
    cerr << "Initial analyses pointers are not null" << endl;
    error = true;
  }

  return error;
}