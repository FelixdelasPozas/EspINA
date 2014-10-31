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
#include <Core/Analysis/Data/MeshData.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Core/IO/SegFile.h>
#include <Core/IO/FetchBehaviour/FetchRawData.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include <testing_support_channel_input.h>
#include <Filters/SeedGrowSegmentationFilter.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;

int pipeline_single_filter_raw_fetch_behaviour( int argc, char** argv )
{
  class TestFilterFactory
  : public FilterFactory
  {
    virtual FilterTypeList providedFilters() const
    {
      FilterTypeList list;
      list << "SGS";
      return list;
    }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
    {
      if (type == "SGS") {
        FilterSPtr filter{new SeedGrowSegmentationFilter(inputs, type, scheduler)};
        filter->setFetchBehaviour(FetchBehaviourSPtr{new FetchRawData()});
        return filter;
      }
    }
  };

  bool error = false;

  CoreFactorySPtr factory{new CoreFactory()};
  factory->registerFilterFactory(FilterFactorySPtr{new TestFilterFactory()});

  Analysis analysis;

  ClassificationSPtr classification{new Classification("Test")};
  classification->createNode("Synapse");
  analysis.setClassification(classification);

  SampleSPtr sample{new Sample("C3P0")};
  analysis.add(sample);

  ChannelSPtr channel(new Channel(Testing::channelInput()));
  channel->setName("channel");

  analysis.add(channel);

  analysis.addRelation(sample, channel, "Stain");

  InputSList inputs;
  inputs << channel->asInput();

  FilterSPtr segFilter{new SeedGrowSegmentationFilter(inputs, "SGS", SchedulerSPtr())};
  segFilter->update();

  auto generateMesh = meshData(segFilter->output(0));

  SegmentationSPtr segmentation(new Segmentation(getInput(segFilter, 0)));
  segmentation->setNumber(1);

  analysis.add(segmentation);

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

  auto loadedSegmentation = analysis2->segmentations().first();
  auto loadedOuptut       = loadedSegmentation->output();
  auto loadedFilter       = dynamic_cast<SeedGrowSegmentationFilter*>(loadedOuptut->filter());

  if (!loadedFilter)
  {
    cerr << "Couldn't recover SGS filter" << endl;
    error = true;
  }

  if (!loadedOuptut->hasData(VolumetricData<itkVolumeType>::TYPE))
  {
    cerr << "Expected Volumetric Data" << endl;
    error = true;
  }
  else
  {
    auto volume = volumetricData(loadedOuptut);

    if (volume->editedRegions().size() != 0)
    {
      cerr << "Unexpeceted number of edited regions" << endl;
      error = true;
    }

    TemporalStorageSPtr tmpStorage(new TemporalStorage());
    for (auto snapshot : volume->snapshot(tmpStorage, "segmentation", "1"))
    {
      if (snapshot.first.contains("EditedRegion"))
      {
        cerr << "Unexpected edited region found" << snapshot.first.toStdString() << endl;
        error = true;
      }
    }
  }

  if (!loadedOuptut->hasData(MeshData::TYPE))
  {
    cerr << "Expected Mesh Data" << endl;
    error = true;
  }
  else
  {
    auto mesh = meshData(loadedOuptut);

    if (!mesh->mesh())
    {
      cerr << "Expected Mesh Data Polydata" << endl;
      error = true;
    }
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
