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
#include <Core/MultiTasking/Scheduler.h>
#include <Core/IO/SegFile.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include "io_testing_support.h"

using namespace EspINA;
using namespace EspINA::IO;
using namespace std;

bool operator!=(Analysis &lhs, Analysis &rhs)
{
  if (print(lhs.classification()) != print(rhs.classification()))
  {
    cerr << "Classification missmatch" << endl;
    return true;
  }

  if (lhs.samples().size() != rhs.samples().size())
  {
    cerr << "Samples size missmatch" << endl;
    return true;
  }

  if (lhs.channels().size() != rhs.channels().size())
  {
    cerr << "Channels size missmatch" << endl;
    return true;
  }

  if (lhs.segmentations().size() != rhs.segmentations().size())
  {
    cerr << "Segmentations size missmatch" << endl;
    return true;
  }

  if (lhs.extensionProviders().size() != rhs.extensionProviders().size())
  {
    cerr << "Extension Providers size missmatch" << endl;
    return true;
  }

  if (lhs.content()->vertices().size() != rhs.content()->vertices().size())
  {
    cerr << "Content vertices size missmatch" << endl;
    return true;
  }

  if (lhs.content()->edges().size() != rhs.content()->edges().size())
  {
    cerr << "Contetn edges size missmatch" << endl;
    return true;
  }

  if (lhs.relationships()->vertices().size() != rhs.relationships()->vertices().size())
  {
    cerr << "Relationships vertices size missmatch" << endl;
    return true;
  }

  if (lhs.relationships()->edges().size() != rhs.relationships()->edges().size())
  {
    cerr << "Relationships edges size missmatch" << endl;
    return true;
  }

  return false;
}

int io_analysis_seg_file_io( int argc, char** argv )
{
  class DummyFilterFactory
  : public FilterFactory
  {
    virtual FilterTypeList providedFilters() const
    { FilterTypeList list; list << "DummyFilter"; return list; }
    virtual FilterSPtr createFilter(OutputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
    {
      return FilterSPtr{new IO_Testing::DummyFilter()};
    }
  } dummyFactory;

  bool error = false;

  CoreFactorySPtr factory{new CoreFactory()};
  factory->registerFilter(&dummyFactory);

  Analysis analysis;

  ClassificationSPtr classification{new Classification("Test")};
  analysis.setClassification(classification);

  FilterSPtr filter{new IO_Testing::DummyFilter()};
  ChannelSPtr channel(new Channel(filter, 0));
  channel->setName("channel");

  analysis.add(channel);

  SampleSPtr sample{new Sample("C3P0")};
  analysis.add(sample);

  analysis.addRelation(sample, channel, "Stain");

  QFileInfo file("analysis.seg");
  try {
    SegFile::save(&analysis, file);
  }
  catch (SegFile::IO_Error_Exception e) {
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

  if (analysis != *(analysis2.get()))
  {
    cerr << "Loaded analysis don't match saved analysis" << endl;
    error = true;
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}