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
#include <GUI/Model/Proxies/ChannelProxy.h>
#include <GUI/ModelFactory.h>

#include "channel_proxy_testing_support.h"
#include "ModelTest.h"
#include "ModelTestUtils.h"

using namespace std;
using namespace ESPINA;
using namespace Testing;

int channel_proxy_add_segmentations( int argc, char** argv )
{
  bool error = false;

  auto modelAdapter = make_shared<ModelAdapter>();
  auto coreFactory  = make_shared<CoreFactory>();

  ModelFactory factory(coreFactory);

  ChannelProxy proxy(modelAdapter);
  ModelTest    modelTester(&proxy);

  InputSList inputs;
  Filter::Type type{"DummyFilter"};

  auto filter = factory.createFilter<DummyFilter>(inputs, type);

  SegmentationAdapterSList segmentations;
  segmentations << factory.createSegmentation(filter, 0)
                << factory.createSegmentation(filter, 0)
                << factory.createSegmentation(filter, 0);

  modelAdapter->add(segmentations);

  error |= checkRowCount(proxy, 0);

  return error;
}