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

#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/IO/SegFile.h>
#include <Core/Factory/CoreFactory.h>

#include "testing.h"
#include <Filters/SeedGrowSegmentationFilter.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;

int seed_grow_segmentation_change_spacing_restore_pipeline( int argc, char** argv )
{
  bool error = false;

  SchedulerSPtr scheduler;

  auto factory = std::make_shared<CoreFactory>();
  factory->registerFilterFactory(std::make_shared<TestFilterFactory>());

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

  QFileInfo file("analysis.seg");
  try {
    SegFile::save(&analysis, file);
  }
  catch (SegFile::IO_Error_Exception &e) {
    cerr << "Couldn't save seg file" << endl;
    error = true;
  }

  auto analysis2 = loadAnalyisis(file, factory);

  analysis2->changeSpacing(analysis2->channels()[0], NmVector3{4,2,4});

  if (analysis2)
  {
    auto checkRestore = [](SegmentationSPtr segmentation)
    {
      auto output = segmentation->output();
      auto filter = dynamic_cast<SeedGrowSegmentationFilter*>(output->filter());

      cerr << "Update Filter" << endl;
      filter->setThreshold(40);
      filter->update();

      return false;
    };

    error |= checkSegmentations(analysis2, 1)
          || checkFilterType<SeedGrowSegmentationFilter>(analysis2->segmentations()[0])
          || checkRestore(analysis2->segmentations()[0])
          || checkValidData(analysis2->segmentations()[0], 0);
  }
  else
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
