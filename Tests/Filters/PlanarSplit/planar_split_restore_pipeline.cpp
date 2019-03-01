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
#include <Core/Utils/EspinaException.h>

#include "testing.h"
#include <Filters/DilateFilter.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;

int planar_split_restore_pipeline( int argc, char** argv )
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

  auto splitSegmentations = split(segmentation1);

  auto segmentation2 = splitSegmentations[0];
  auto segmentation3 = splitSegmentations[1];

  error |= dilate(segmentation2);
  error |= dilate(segmentation3);

  analysis.remove(segmentation1);
  analysis.add(segmentation2);
  analysis.add(segmentation3);

  QFileInfo file("analysis.seg");
  try
  {
    SegFile::save(&analysis, file);
  }
  catch (const EspinaException &e)
  {
    cerr << "Couldn't save seg file" << endl;
    error = true;
  }

  auto analysis2 = loadAnalyisis(file, factory);

  if (analysis2)
  {
    auto checkRestore = [](SegmentationSPtr segmentation)
    {
      auto output = segmentation->output();
      auto filter = dynamic_cast<DilateFilter*>(output->filter());

      if(filter)
      {
        filter->setRadius(2);
        filter->update();

        return false;
      }
      return true;
    };

    error |= checkSegmentations(analysis2, 2)
          || checkFilterType<DilateFilter>(analysis2->segmentations()[0])
          || checkFilterType<DilateFilter>(analysis2->segmentations()[1])
          || checkRestore(analysis2->segmentations()[0])
          || checkRestore(analysis2->segmentations()[1])
          || checkValidData(analysis2->segmentations()[0], 0)
          || checkValidData(analysis2->segmentations()[1], 0);
  }
  else
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
