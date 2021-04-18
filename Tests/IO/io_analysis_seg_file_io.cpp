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
#include <Core/MultiTasking/Scheduler.h>
#include <Core/IO/SegFile.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include "io_testing_support.h"
#include "io_testing_dummy_extension.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::IO_Testing;

int io_analysis_seg_file_io( int argc, char** argv )
{
  class DummyFilterFactory
  : public FilterFactory
  {
    virtual const FilterTypeList providedFilters() const
    { FilterTypeList list; list << "DummyFilter"; return list; }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
    { return  std::make_shared<DummyFilter>(); }
  } dummyFactory;

  bool error = false;

  auto factory = make_shared<CoreFactory>();

  Analysis analysis;

  auto classification = make_shared<Classification>("Test");
  analysis.setClassification(classification);

  auto sample = make_shared<Sample>("C3P0");
  analysis.add(sample);

  QString channelName     = "channel";
  QString channelMetadata = "<xml>Test<xml>";

  auto filter  = make_shared<DummyFilter>();
  auto channel = make_shared<Channel>(getInput(filter, 0));
  channel->setName(channelName);
  channel->setMetadata(channelMetadata);

  analysis.add(channel);

  analysis.addRelation(sample, channel, "Stain");

  auto segFilter    = std::make_shared<DummyFilter>();
  auto segmentation = std::make_shared<Segmentation>(getInput(segFilter, 0));
  segmentation->setNumber(1);

  auto extension = std::make_shared<DummySegmentationExtension>();
  segmentation->extensions()->add(extension);

  analysis.add(segmentation);

  QFileInfo file("analysis.seg");
  try
  {
    SegFile::save(&analysis, file);
  }
  catch (...)
  {
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

  auto channel2 = analysis2->channels().first();
  if (channel2->metadata() != channelMetadata) {
    cerr << "Unexpected loaded metadata" << channel2->metadata().toStdString() << endl;
    error = true;
  }

  if (analysis != *(analysis2.get()))
  {
    cerr << "Loaded analysis don't match saved analysis" << endl;
    error = true;
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
