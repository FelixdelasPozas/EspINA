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
#include <Core/Analysis/Output.h>
#include <Tests/Unitary/Core/core_testing_support.h>

using namespace EspINA;
using namespace std;

bool TestHue(ChannelSPtr channel, double hue) {
  bool error = false;
 
  channel->setHue(hue);
  if (channel->hue() != hue) {
    cerr << "Unexepected hue value:" << hue << endl;
    error = true;
  }
  
  return error;
}

//TODO 2013-10-20: Add test to check saturation on hue changes
int channel_set_hue(int argc, char** argv )
{
  bool error = false;

  FilterSPtr filter{new Testing::DummyFilter()};

  ChannelSPtr channel(new Channel(getInput(filter, 0)));
  
  if (channel->hue() != -1) {
    cerr << "Unexepected initial hue value" << endl;
    error = true;
  }
  
  error |= TestHue(channel, -1.0);
  error |= TestHue(channel,  0.0);
  error |= TestHue(channel,  0.5);
  error |= TestHue(channel,  1.0);
  
  return error;
}