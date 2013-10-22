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

using namespace EspINA;
using namespace std;

int channel_delete_extension(int argc, char** argv )
{
  class DummyExtension
  : public ChannelExtension
  {
  public:
    bool Initialized;
    bool ValidChannel;
  public:
    explicit DummyExtension() 
    : Initialized{false}, ValidChannel{false} {}

    virtual void initialize() { Initialized = true; }
    virtual void invalidate() {}
    virtual Type type() const { return "Dummy"; }
  protected:
    virtual void onChannelSet(ChannelPtr channel) { ValidChannel = true; }
  };
  
  bool error = false;

  ChannelSPtr channel(new Channel(FilterSPtr(), 0));

  DummyExtension *dummy = new DummyExtension();
  ChannelExtensionSPtr extension{dummy};
  
  channel->addExtension(extension);
  
  if (!channel->extension(extension->type())) {
    cerr << "Couldn't find expected extension" << endl;
    error = true;
  }

  channel->deleteExtension(extension);
  
  if (channel->extension(extension->type())) {
    cerr << "Unexpected deleted extension" << endl;
    error = true;
  }
  
  return error;
}