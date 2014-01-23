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
#include "analysis_testing_support.h"

using namespace EspINA;
using namespace std;

int analysis_add_channel( int argc, char** argv )
{
  bool error = false;

  Analysis analysis;

  FilterSPtr filter{new Testing::DummyFilter()};
  ChannelSPtr channel(new Channel(filter, 0));

  analysis.add(channel);

  if (!analysis.channels().contains(channel)) {
    cerr << "Analysis doesn't contain addded channel" << endl;
    error = true;
  }

  if (analysis.channels().first() != channel) {
    cerr << "Unexpected channel retrieved from analysis" << endl;
    error = true;
  }

  if (analysis.classification().get() != nullptr) {
    cerr << "Unexpected classification in analysis" << endl;
    error = true;
  }

  if (!analysis.samples().isEmpty()) {
    cerr << "Unexpected number of samples in analysis" << endl;
    error = true;
  }

  if (analysis.channels().size() != 1) {
    cerr << "Unexpected number of channels in analysis" << endl;
    error = true;
  }

  if (!analysis.segmentations().isEmpty()) {
    cerr << "Unexpected number of segmentations in analysis" << endl;
    error = true;
  }

  if (analysis.content()->vertices().size() != 2) {
    cerr << "Unexpected number of vertices in analysis content" << endl;
    error = true;
  }

//   if (analysis.content()->vertices().first().item != channel) {
//     cerr << "Unexpected channel retrieved from analysis content" << endl;
//     error = true;
//   }

  if (analysis.content()->edges().size() != 1) {
    cerr << "Unexpected number of edges in analysis content" << endl;
    error = true;
  }

  if (analysis.relationships()->vertices().size() != 1) {
    cerr << "Unexpected number of vertices in analysis relationships" << endl;
    error = true;
  }

  if (!analysis.relationships()->edges().isEmpty()) {
    cerr << "Unexpected number of edges in analysis relationships" << endl;
    error = true;
  }

  return error;
}