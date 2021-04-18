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
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Filter.h>
#include <Core/MultiTasking/Scheduler.h>

#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/Proxies/ClassificationProxy.h>
#include <GUI/ModelFactory.h>
#include <GUI/View/ViewState.h>

#include "classification_proxy_testing_support.h"
#include "ModelTest.h"
#include "ModelProfiler.h"
#include <QElapsedTimer>
#include <QElapsedTimer>

using namespace std;
using namespace ESPINA;
using namespace Testing;
using ViewState = GUI::View::ViewState;


namespace CPADCS
{
  const unsigned NUM_CAT        = 3;
  const unsigned NUM_SEG_BY_CAT = 100;

  using CategorySegmentations = QList<SegmentationAdapterSList>;

  ModelFactory factory(make_shared<CoreFactory>());

  InputSList inputs;
  Filter::Type type{"DummyFilter"};

  auto filter = factory.createFilter<DummyFilter>(inputs, type);

  CategorySegmentations createCategorySegmentations(ModelAdapterSPtr model)
  {
    CategorySegmentations segsByCat;

    for (unsigned i = 0; i < NUM_CAT; ++i)
    {
      auto category = model->createRootCategory(QString("Category %1").arg(i));

      SegmentationAdapterSList segmentations;

      for (unsigned j = 0; j < NUM_SEG_BY_CAT; ++j)
      {
        auto segmentation = factory.createSegmentation(filter, 0);
        segmentation->setCategory(category);
        segmentation->setNumber(j);

        segmentations << segmentation;
      }

      segsByCat << segmentations;
    }

    return segsByCat;
  }


  void addInterlacedSegmentations(ModelAdapterSPtr model, CategorySegmentations segsByCat)
  {
    for (unsigned i = 0; i < NUM_SEG_BY_CAT; ++i)
    {
      for (auto &catSeg : segsByCat)
      {
        model->add(catSeg[i]);
      }
    }
  }


  void removeSegmentations(ModelAdapterSPtr model, CategorySegmentations segsByCat)
  {
    for (auto &catSeg : segsByCat)
    {
      model->remove(catSeg);
    }
  }
}

using namespace CPADCS;

int classification_proxy_add_different_category_segmentations( int argc, char** argv )
{
  bool error = false;

  ViewState           viewState;
  ModelAdapterSPtr    modelAdapter(new ModelAdapter());
  ClassificationProxy proxy(modelAdapter, viewState);
  //ModelTest           modelTester(&proxy);

  auto classification = make_shared<ClassificationAdapter>();
  classification->setName("Test");
  modelAdapter->setClassification(classification);

  auto segsByCat = createCategorySegmentations(modelAdapter);

  ModelProfiler modelProfiler(*modelAdapter);
  ModelProfiler proxyProfiler(proxy);

  const unsigned NUM_SEGS          = NUM_SEG_BY_CAT*NUM_CAT;
  // The last removal is consecutive because all interlazed segmentation have been removed
  const unsigned NUM_NORMAL_RATBRS = NUM_SEG_BY_CAT*(NUM_CAT - 1) + 1;

  QElapsedTimer timer;
  timer.start();
  addInterlacedSegmentations(modelAdapter, segsByCat);
  cout << "Normal Mode Addition Time: " << timer.elapsed() << " ms" << endl;
  error |= checkExpectedNumberOfSignals(modelProfiler, NUM_SEGS, 0, 0, 0);
  error |= checkExpectedNumberOfSignals(proxyProfiler, NUM_SEGS, 0, 0, 0);

  modelProfiler.reset();
  proxyProfiler.reset();

  timer.start();
  removeSegmentations(modelAdapter, segsByCat);
  cout << "Normal Mode Remove Time: " << timer.elapsed() << " ms" << endl;
  error |= checkExpectedNumberOfSignals(modelProfiler, 0, 0, 0, NUM_NORMAL_RATBRS);
  error |= checkExpectedNumberOfSignals(proxyProfiler, 0, 0, 0, NUM_NORMAL_RATBRS);

  modelProfiler.reset();
  proxyProfiler.reset();

  timer.start();
  modelAdapter->beginBatchMode();
  addInterlacedSegmentations(modelAdapter, segsByCat);
  modelAdapter->endBatchMode();
  cout << "Batch Mode Addition Time: " << timer.elapsed() << " ms" << endl;
  error |= checkExpectedNumberOfSignals(modelProfiler, 1, 0, 0, 0);
  error |= checkExpectedNumberOfSignals(proxyProfiler, NUM_CAT, 0, 0, 0);

  modelProfiler.reset();
  proxyProfiler.reset();

  timer.start();
  modelAdapter->beginBatchMode();
  removeSegmentations(modelAdapter, segsByCat);
  modelAdapter->endBatchMode();
  cout << "Batch Mode Remove Time: " << timer.elapsed() << " ms" << endl;
  error |= checkExpectedNumberOfSignals(modelProfiler, 0, 0, 0, 1);
  error |= checkExpectedNumberOfSignals(proxyProfiler, 0, 0, 0, NUM_CAT);

  return error;
}