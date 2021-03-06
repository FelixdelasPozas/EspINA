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
#include <Tests/Core/core_testing_support.h>

using namespace ESPINA;
using namespace std;

int channel_add_extension(int argc, char** argv )
{
  class DummyExtension
  : public Core::StackExtension
  {
  public:
    bool Initialized;
    bool ValidChannel;
  public:
    explicit DummyExtension() 
    : Core::StackExtension(InfoCache()), Initialized{false}, ValidChannel{false} {}

    virtual const InformationKeyList availableInformation() const { return InformationKeyList(); }
    virtual QVariant cacheFail(const InformationKey& tag) const { return QVariant(); }
    virtual const TypeList dependencies() const {return TypeList(); }
    virtual bool invalidateOnChange() const {return false;}
    virtual Snapshot snapshot() const {return Snapshot();}
    virtual State state() const {return State();}
    virtual void initialize() { Initialized = true; }
    virtual void invalidate() {}
    virtual Type type() const { return "Dummy"; }
  protected:
    virtual void onExtendedItemSet(Channel* item) {ValidChannel = true; }
  };

  bool error = false;

  auto filter    = std::make_shared<Testing::DummyFilter>();
  auto channel   = std::make_shared<Channel>(getInput(filter, 0));
  auto extension = std::make_shared<DummyExtension>();

  auto extensions = channel->extensions();

  if (extensions->hasExtension(extension->type())) {
    cerr << "Unexepected initial extension" << endl;
    error = true;
  }

  extensions->add(extension);

  if (!extensions->hasExtension(extension->type())) {
    cerr << "Couldn't find expected extension" << endl;
    error = true;
  }

  if (!extension->ValidChannel) {
    cerr << "Extension was not correctly initialized" << endl;
    error = true;
  }

  return error;
}
