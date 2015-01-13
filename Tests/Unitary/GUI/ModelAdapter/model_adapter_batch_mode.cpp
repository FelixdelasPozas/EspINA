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
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include "ModelTest.h"
#include "ModelProfiler.h"
#include "model_adapter_testing_support.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

void populateModel( ModelAdapter      &modelAdapter,
                    ModelFactorySPtr   factory,
                    FilterSPtr         filter,
                    SampleAdapterSPtr  sample1,
                    SampleAdapterSPtr  sample2,
                    SampleAdapterSPtr  sample3,
                    ChannelAdapterSPtr channel1)
{
  auto sample4 = factory->createSample("Sample 4");

  modelAdapter.addRelation(sample2, sample3, "link");

  modelAdapter.add(sample4);
  modelAdapter.addRelation(sample1, sample4, "link");

  auto channel2 = factory->createChannel(filter, 0);
  modelAdapter.add(channel2);
  modelAdapter.addRelation(channel1, channel2, "link");

  modelAdapter.addRelation(channel1, sample2, "link");
  modelAdapter.addRelation(sample1, channel2, "link");

  auto segmentation1 = factory->createSegmentation(filter, 0);
  auto segmentation2 = factory->createSegmentation(filter, 0);
  modelAdapter.add(segmentation1);
  modelAdapter.add(segmentation2);
  modelAdapter.addRelation(segmentation1, segmentation2, "link");

  SegmentationAdapterSList segmentationSet;

  for (int i = 0; i < 3; ++i)
  {
    auto segmentation = factory->createSegmentation(filter, 0);

    segmentationSet << segmentation;
  }

  modelAdapter.add(segmentationSet);

  modelAdapter.remove(channel1);

  modelAdapter.remove(segmentationSet);

}

int model_adapter_batch_mode(int argc, char** argv)
{
  bool error = false;


  auto analysis    = make_shared<Analysis>();
  auto coreFactory = make_shared<CoreFactory>();
  auto factory     = make_shared<ModelFactory>(coreFactory);

  InputSList inputs;
  Filter::Type type{"DummyFilter"};
  auto filter  = factory->createFilter<DummyFilter>(inputs, type);


  ModelAdapter  modelAdapter;
  ModelTest     modelTester(&modelAdapter);
  ModelProfiler modelProfiler(modelAdapter);

  auto sample1 = factory->createSample("Sample 1");
  auto sample2 = factory->createSample("Sample 2");
  auto sample3 = factory->createSample("Sample 3");
  modelAdapter.add(sample1);
  modelAdapter.add(sample2);
  modelAdapter.add(sample3);

  auto channel1 = factory->createChannel(filter, 0);
  modelAdapter.add(channel1);
  modelProfiler.reset();

  cout << "Normal Mode Execution" << endl;
  populateModel(modelAdapter, factory, filter, sample1, sample2, sample3, channel1);
  error |= checkExpectedNumberOfSignals(modelProfiler, 5, 9, 2); // 1 add for each add
                                                                  // 2 update for each non consecutive item relation, 1 for consecutive
                                                                  // 1 remove for each remove

  modelAdapter.clear();
  modelAdapter.add(sample1);
  modelAdapter.add(sample2);
  modelAdapter.add(sample3);
  modelAdapter.add(channel1);
  modelProfiler.reset();

  cout << "Batch Mode Execution" << endl;
  modelAdapter.beginBatchMode();
  populateModel(modelAdapter, factory, filter, sample1, sample2, sample3, channel1);
  modelAdapter.endBatchMode();
  error |= checkExpectedNumberOfSignals(modelProfiler, 3, 1, 1); // 1 add for each type minus one for the remove
                                                                 // 1 update for all existing samples and no update for channel1 since its removed
                                                                 // 1 remove for the removal of an already added item

  return error;
}