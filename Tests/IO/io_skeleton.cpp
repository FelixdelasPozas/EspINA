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
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/IO/SegFile.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include <Filters/SourceFilter.h>
#include <Plugins/AppositionSurface/Filter/AppositionSurfaceFilter.h>
#include <Plugins/AppositionSurface/Filter/SASDataFactory.h>
#include "testing_support_channel_input.h"
#include "SkeletonTestingUtils.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::Testing;

int io_skeleton( int argc, char** argv )
{
  class TestFilterFactory
  : public FilterFactory
  {
    virtual const FilterTypeList providedFilters() const
    {
      FilterTypeList list;
      list << "Skeleton";
      return list;
    }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
    {
      FilterSPtr filter;

      if (type == "Skeleton") {
        filter = std::make_shared<SourceFilter>(inputs, type, scheduler);
        filter->setDataFactory(std::make_shared<RawDataFactory>());
      }

      return filter;
    }
  };

  bool error = false;

  auto factory = make_shared<CoreFactory>();
  factory->registerFilterFactory(make_shared<TestFilterFactory>());

  Analysis analysis;

  auto classification = make_shared<Classification>("Test");
  classification->createNode("Synapse");
  analysis.setClassification(classification);

  auto sample = make_shared<Sample>("C3P0");
  analysis.add(sample);

  auto channel = make_shared<Channel>(Testing::channelInput());
  channel->setName("channel");

  analysis.add(channel);

  analysis.addRelation(sample, channel, "Stain");

  auto sourceFilter = make_shared<SourceFilter>(InputSList(), "Skeleton", SchedulerSPtr());

  auto output   = make_shared<Output>(sourceFilter.get(), 0, NmVector3{1,1,1});
  auto skeleton = make_shared<RawSkeleton>(createSimpleTestSkeleton());

  output->setData(skeleton);
  sourceFilter->addOutput(0, output);


  auto segmentation = make_shared<Segmentation>(getInput(sourceFilter, 0));
  segmentation->setNumber(1);

  analysis.add(segmentation);

  QFileInfo file("analysis.seg");
  try {
    SegFile::save(&analysis, file);
  }
  catch (SegFile::IO_Error_Exception &e) {
    cerr << "Couldn't save seg file" << endl;
    error = true;
  }

  AnalysisSPtr analysis2;
  try
  {
   analysis2 = SegFile::load(file, factory);
  } catch (...)
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  auto loadedSegmentation = analysis2->segmentations().first();
  auto loadedOuptut       = loadedSegmentation->output();
  auto loadedFilter       = dynamic_cast<SourceFilter *>(loadedOuptut->filter());

  if (!loadedFilter)
  {
    cerr << "Couldn't recover Source Filter" << endl;
    error = true;
  }

  if (!loadedOuptut->hasData(SkeletonData::TYPE))
  {
    cerr << "Expected Skeleton Data" << endl;
    error = true;
  }
  else
  {
    auto loadedSkeleton = readLockSkeleton(loadedOuptut);
    
    if (!loadedSkeleton->isValid())
    {
      cerr << "Unexpeceted invalid skeleton data" << endl;
      error = true;
    }

    if (loadedSkeleton->editedRegions().size() != 1)
    {
      cerr << "Unexpeceted number of edited regions" << loadedSkeleton->editedRegions().size() << endl;
      error = true;
    }
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
