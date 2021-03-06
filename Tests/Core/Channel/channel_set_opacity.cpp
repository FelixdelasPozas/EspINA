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

#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Output.h"
#include <Tests/Core/core_testing_support.h>

using namespace ESPINA;
using namespace std;

bool TestOpacity(ChannelSPtr channel, double opacity) {
  bool error = false;
 
  channel->setOpacity(opacity);
  if (channel->opacity() != abs(opacity))
  {
    cerr << "Unexepected opacity value:" << opacity << endl;
    error = true;
  }
  
  return error;
}

int channel_set_opacity(int argc, char** argv )
{
  bool error = false;

  FilterSPtr filter{new Testing::DummyFilter()};

  ChannelSPtr channel(new Channel(getInput(filter, 0)));
  
  if (channel->opacity() != 1.0) {
    cerr << "Unexepected initial opacity value" << endl;
    error = true;
  }
  
  error |= TestOpacity(channel, -1.0); // This should change to 1.0
  error |= TestOpacity(channel,  0.0);
  error |= TestOpacity(channel,  0.5);
  error |= TestOpacity(channel,  1.0);
  
  return error;
}