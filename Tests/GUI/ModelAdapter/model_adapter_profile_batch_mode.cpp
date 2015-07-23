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
#include "testing_support_dummy_filter.h"
#include <QElapsedTimer>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;


const unsigned NUM_SAMPLES   = 100;
const unsigned NUM_RELATIONS = 2;

namespace MAPBB
{
  void profileModel(ModelAdapter       &modelAdapter,
                    ModelFactorySPtr   factory,
                    FilterSPtr         filter,
                    SampleAdapterSList initialSamples)
  {
    SampleAdapterSList newSamples;
    for (unsigned i = 0; i < NUM_SAMPLES; ++i)
    {
      auto sample = factory->createSample(QString("New Sample %1").arg(i));
      modelAdapter.add(sample);

      for (unsigned j = 0; j < NUM_RELATIONS; ++j) {
        modelAdapter.addRelation(initialSamples[i], sample, QString("link %1").arg(j));
      }
    }

    // Remove on inverse order
    for (int i = NUM_SAMPLES - 1; i >= 0; --i)
    {
      modelAdapter.remove(initialSamples[i]);
    }
  }
}

using namespace MAPBB;

int model_adapter_profile_batch_mode(int argc, char** argv)
{
  bool error = false;

  auto analysis    = make_shared<Analysis>();
  auto coreFactory = make_shared<CoreFactory>();
  auto factory     = make_shared<ModelFactory>(coreFactory);

  InputSList inputs;
  Filter::Type type{"DummyFilter"};
  auto filter  = factory->createFilter<DummyFilter>(inputs, type);

  ModelAdapter  modelAdapter;
  ModelProfiler modelProfiler(modelAdapter);

  SampleAdapterSList initialSamples;
  for (unsigned i = 0; i < NUM_SAMPLES; ++i)
  {
    initialSamples << factory->createSample(QString("Sample %1").arg(i));
  }

  QElapsedTimer t;
  t.start();
  modelAdapter.add(initialSamples);
  modelProfiler.reset();
  cout << "Initial Population Time: " << t.elapsed() << " ms" << endl;

  t.start();
  profileModel(modelAdapter, factory, filter, initialSamples);
  cout << "Normal Mode Execution Time: " << t.elapsed() << " ms" << endl;
  // 1 add for each add
  // 2 update for each non consecutive item relation, 1 for consecutive
  // 0
  // 1 add for each remove
  error |= checkExpectedNumberOfSignals(modelProfiler, NUM_SAMPLES, 2*NUM_RELATIONS*NUM_SAMPLES, 0, NUM_SAMPLES);

  t.start();
  modelAdapter.clear();
  modelAdapter.add(initialSamples);
  modelProfiler.reset();
  cout << "Reset Time: " << t.elapsed() << " ms" << endl;

  t.start();
  modelAdapter.beginBatchMode();
  profileModel(modelAdapter, factory, filter, initialSamples);
  modelAdapter.endBatchMode();
  cout << "Batch Mode Execution Time: " << t.elapsed() << " ms" << endl;
  // 1 add for each type minus one for the remove
  // 1 update for all existing samples
  // 0
  // 1 remove for the removal of an already added consecutive items
  error |= checkExpectedNumberOfSignals(modelProfiler, 1, 0, 0, 1);

  return error;
}