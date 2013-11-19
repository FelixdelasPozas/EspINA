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

#include "Core/Analysis/Analysis.h"
#include <GUI/Model/ClassificationAdapter.h>

#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/Proxies/ClassificationProxy.h>
#include "ModelTest.h"

using namespace std;
using namespace EspINA;

int classification_proxy_set_classification( int argc, char** argv )
{
  bool error = false;

  ModelAdapterSPtr    modelAdapter(new ModelAdapter());
  ClassificationProxy proxy(modelAdapter);
  ModelTest           modelTester(&proxy);

  ClassificationAdapterSPtr classification{new ClassificationAdapter()};
  classification->setName("Test");
  classification->createCategory("Level 1/Level 2");

  modelAdapter->setClassification(classification);

  if (proxy.rowCount() != 1) {
    cerr << "Unexpected number of root categories" << endl;
    error = true;
  }

  auto level1 = proxy.index(0, 0);
  if (level1.data(Qt::DisplayRole) != "Level 1") {
    cerr << "Unexpected display role value: " << level1.data(Qt::DisplayRole).toString().toStdString() << endl;
    error = true;
  }

  if (proxy.rowCount(level1) != 1) {
    cerr << "Unexpected number of level 1 sub-categories" << endl;
    error = true;
  }

  return error;
}