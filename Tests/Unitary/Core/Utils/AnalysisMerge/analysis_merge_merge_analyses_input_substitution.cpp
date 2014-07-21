/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Tests/Unitary/Core/Utils/AnalysisMerge/analysis_merge_testing_support.h>

using namespace std;
using namespace EspINA;
using namespace EspINA::Testing;

int analysis_merge_merge_analyses_input_substitution( int argc, char** argv )
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
  analysis1->addRelation(sample1, channel1, Channel::STAIN_LINK);

  for(auto i: {0,1,2,3,4})
  {
    FilterSPtr segmentationFilter1{new DummyFilter(inputs1)};
    auto inputS1 = getInput(segmentationFilter1, 0);

    SegmentationSPtr segmentation1{new Segmentation(inputS1)};
    segmentation1->setName("Segmentation " + QString::number(i));

    analysis1->add(segmentation1);
  }

  AnalysisSPtr analysis2{new Analysis()};

  SampleSPtr sample2{new Sample("Sample")};

  FilterSPtr channelFilter2{new DummyFilter()};
  auto inputC2 = getInput(channelFilter2, 0);

  ChannelSPtr channel2(new Channel(inputC2));
  channel2->setName("Channel 1");

  InputSList inputs2;
  inputs2 << inputC2;

  analysis2->add(sample2);
  analysis2->add(channel2);
  analysis2->addRelation(sample2, channel2, Channel::STAIN_LINK);

  for(auto i: {5,6,7,8,9})
  {
    FilterSPtr segmentationFilter2{new DummyFilter(inputs2)};
    auto inputS2 = getInput(segmentationFilter2, 0);

    SegmentationSPtr segmentation2{new Segmentation(inputS2)};
    segmentation2->setName("Segmentation " + QString::number(i));

    analysis2->add(segmentation2);
  }

  AnalysisSPtr merged = merge(analysis1, analysis2);

  if(merged->content()->successors(merged->channels().first()->filter().get()).size() != 11)
  {
    cerr << "Unexpected number of successors in merged channel." << endl;
    error = true;
  }

  auto channelInput = merged->channels().first()->asInput();

  for(auto segmentation: merged->segmentations())
  {
    if((segmentation->filter()->inputs().first()->filter() != channelInput->filter()) ||
        segmentation->filter()->inputs().first()->output() != channelInput->output())
    {
      cerr << "Failed input substitution in merge process." << endl;
      error = true;
    }
  }

  return error;
}
