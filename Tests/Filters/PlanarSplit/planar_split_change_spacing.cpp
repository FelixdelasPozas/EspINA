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
#include <Filters/SplitFilter.h>
#include <testing_support_channel_input.h>
#include "testing.h"

#include <vtkImageStencilData.h>


using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;

int planar_split_change_spacing( int argc, char** argv )
{
  Analysis analysis;

  auto classification = std::make_shared<Classification>("Test");
  classification->createNode("Synapse");
  analysis.setClassification(classification);

  auto sample = std::make_shared<Sample>("C3P0");
  analysis.add(sample);

  auto channel = std::make_shared<Channel>(Testing::channelInput());
  channel->setName("channel");

  analysis.add(channel);

  analysis.addRelation(sample, channel, "Stain");

  auto segmentation1 = gls(channel);
  analysis.add(segmentation1);

  auto splitSegmentations = split(segmentation1);

  auto segmentation2 = splitSegmentations[0];
  auto segmentation3 = splitSegmentations[1];

//   {
//     cerr << "Before" << endl;
//     auto volume2 = readLockVolume(segmentation2->output());
//     auto mesh2   = readLockMesh(segmentation2->output());
//
//     cerr << segmentation2->output()->bounds() << endl;
//     cerr << volume2->bounds() << endl;
//     cerr << mesh2->bounds() << endl;
//   }

  analysis.remove(segmentation1);
  analysis.add(segmentation2);
  analysis.add(segmentation3);

  auto split = dynamic_cast<SplitFilter*>(segmentation2->filter().get());

  NmVector3 prevStencil(split->stencil()->GetSpacing());

  analysis.changeSpacing(channel, NmVector3{4,2,4});

//   {
//     cerr << "After" << endl;
//     auto volume2 = readLockVolume(segmentation2->output());
//     auto mesh2   = readLockMesh(segmentation2->output());
//
//     cerr << segmentation2->output()->bounds() << endl;
//     cerr << volume2->bounds() << endl;
//     cerr << mesh2->bounds() << endl;
//   }

  NmVector3 stencil(split->stencil()->GetSpacing());

  return checkSplitBounds(segmentation1, segmentation2, segmentation3)
      || !checkSpacing(prevStencil, stencil)
      || checkSpacing(stencil, NmVector3{4,2,4});
}