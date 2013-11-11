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
#include "ModelTest.h"

using namespace EspINA;
using namespace std;

int model_adapter_set_classification( int argc, char** argv )
{
  bool error = false;

  AnalysisSPtr analysis{new Analysis()};
  ModelAdapter modelAdapter(analysis);
  ModelTest    modelTester(&modelAdapter);

  ClassificationAdapterSPtr classification{new ClassificationAdapter()};
  classification->setName("Test");
  classification->createCategory("Level 1/Level 2");

  modelAdapter.setClassification(classification);

  if (analysis->classification()->name() != classification->name()) {
    cerr << "Unexpected classification name in analysis" << endl;
    error = true;
  }

  if (!analysis->samples().isEmpty()) {
    cerr << "Unexpected number of samples in analysis" << endl;
    error = true;
  }

  if (!analysis->channels().isEmpty()) {
    cerr << "Unexpected number of channels in analysis" << endl;
    error = true;
  }

  if (!analysis->segmentations().isEmpty()) {
    cerr << "Unexpected number of segmentations in analysis" << endl;
    error = true;
  }

  if (!analysis->extensionProviders().isEmpty()) {
    cerr << "Unexpected number of extension providers in analysis" << endl;
    error = true;
  }

  if (!analysis->content()->vertices().isEmpty()) {
    cerr << "Unexpected number of vertices in analysis content" << endl;
    error = true;
  }

  if (!analysis->content()->edges().isEmpty()) {
    cerr << "Unexpected number of edges in analysis content" << endl;
    error = true;
  }

  if (!analysis->relationships()->vertices().isEmpty()) {
    cerr << "Unexpected number of vertices in analysis relationships" << endl;
    error = true;
  }

  if (!analysis->relationships()->edges().isEmpty()) {
    cerr << "Unexpected number of edges in analysis relationships" << endl;
    error = true;
  }

  return error;
}