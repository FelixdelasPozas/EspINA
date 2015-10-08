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

using namespace std;
using namespace ESPINA;
using namespace Testing;
using ViewState = GUI::View::ViewState;

int classification_proxy_add_categories( int argc, char** argv )
{
  bool error = false;

  ViewState           viewState;
  ModelAdapterSPtr    modelAdapter(new ModelAdapter());
  ClassificationProxy proxy(modelAdapter, viewState.representationInvalidator());
  ModelTest           modelTester(&proxy);

  auto classification = make_shared<ClassificationAdapter>();

  modelAdapter->setClassification(classification);

  auto l1 = modelAdapter->createRootCategory("Level 1");

  if (proxy.rowCount() != 1) {
    cerr << "Unexpected number of root categories" << endl;
    error = true;
  }

  auto level1 = proxy.index(0, 0);

  if (proxy.rowCount(level1) != 0) {
    cerr << "Unexpected number of level 1 row count" << endl;
    error = true;
  }

  auto l1_1 = modelAdapter->createCategory("Level 1-1", l1);

  if (proxy.rowCount() != 1) {
    cerr << "Unexpected number of root categories" << endl;
    error = true;
  }

  if (proxy.rowCount(level1) != 1) {
    cerr << "Unexpected number of level 1 row count" << endl;
    error = true;
  }

  auto l2 = modelAdapter->createRootCategory("Level 2");

  if (proxy.rowCount() != 2) {
    cerr << "Unexpected number of root categories" << endl;
    error = true;
  }

  return error;
}