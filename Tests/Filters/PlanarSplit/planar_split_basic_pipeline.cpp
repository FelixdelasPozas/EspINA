/*
 * Copyright (c) 2015, Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/MeshData.h>
#include <testing_support_channel_input.h>

#include "testing.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;

int planar_split_basic_pipeline( int argc, char** argv )
{
  bool error = false;

  auto channel = std::make_shared<Channel>(Testing::channelInput());
  channel->setName("channel");

  auto segmentations = gls_split(channel);

  auto bounds1 = segmentations[0]->bounds();
  auto bounds2 = segmentations[1]->bounds();
  auto bounds3 = segmentations[2]->bounds();

  for (auto i : {0, 1, 4, 5})
  {
    if ( bounds1[i] != bounds2[i]
      || bounds1[i] != bounds3[i])
    {
      std::cerr << "Incorrect bounds on dimension " << i << std::endl;
      error = true;
    }
  }

  auto halfBounds1 = (bounds1[3] + bounds1[2]) / 2;
  if ( halfBounds1 != bounds2[3]
    || halfBounds1 != bounds3[2])
  {
    std::cerr << "Incorrect bounds on split dimension " << std::endl;
    error = true;
  }

  return error;
}
