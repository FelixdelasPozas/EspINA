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
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Factory/CoreFactory.h>
#include <Filters/DilateFilter.h>
#include <Filters/SeedGrowSegmentationFilter.h>

#include "Tests/testing_support_channel_input.h"
#include "Tests/Testing_Support.h"

#include <itkImageConstIterator.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;
using namespace ESPINA::IO;

using ConstIterator = itk::ImageRegionConstIterator<itkVolumeType>;

int pipeline_update_filter_with_no_edited_region_filter_as_input(int argc, char** argv)
{
  class TestFilterFactory
  : public FilterFactory
  {
    virtual const FilterTypeList providedFilters() const
    {
      FilterTypeList list;
      list << "DummyChannelReader" << "SGS" << "Dilate";
      return list;
    }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const
    {
      FilterSPtr filter;

      if (type == "DummyChannelReader") {
        filter = make_shared<DummyChannelReader>();
      }
      else
      {
        if (type == "SGS")
        {
          filter = make_shared<SeedGrowSegmentationFilter>(inputs, type, scheduler);
        }
        else if (type == "Dilate")
        {
          filter = make_shared<DilateFilter>(inputs, type, scheduler);
        } else
        {
          Q_ASSERT(false);
        }
        filter->setDataFactory(make_shared<MarchingCubesFromFetchedVolumetricData>());
      }

      return filter;
    }
  };

  bool error = false;

  auto factory = std::make_shared<CoreFactory>();
  factory->registerFilterFactory(make_shared<TestFilterFactory>());

  Analysis analysis;

  auto classification = make_shared<Classification>("Test");
  classification->createNode("Synapse");
  analysis.setClassification(classification);

  auto sample = make_shared<Sample>("C3P0");
  analysis.add(sample);

  auto channel = make_shared<Channel>(channelInput());
  channel->setName("channel");

  analysis.add(channel);

  analysis.addRelation(sample, channel, "Stain");

  InputSList inputs;
  inputs << channel->asInput();

  SchedulerSPtr scheduler;

  auto sgs = make_shared<SeedGrowSegmentationFilter>(inputs, "SGS", scheduler);
  sgs->update();

  auto dilate = make_shared<DilateFilter>(getInputs(sgs), "Dilate", scheduler);
  dilate->setRadius(1);
  dilate->update();

  auto segmentation = make_shared<Segmentation>(getInput(dilate, 0));
  segmentation->setNumber(1);

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
  }
  catch (...)
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  auto loadedSegmentation = analysis2->segmentations().first();
  auto loadedDilateOuptut = loadedSegmentation->output();
  auto loadedDilateFilter = dynamic_cast<DilateFilter*>(loadedDilateOuptut->filter());
  if (!loadedDilateFilter)
  {
    cerr << "Couldn't recover Dilate filter" << endl;
    error = true;
  }

  loadedDilateFilter->setRadius(2);
  loadedDilateFilter->update();

  auto sgsVolume = readLockVolume(loadedDilateFilter->inputs()[0]->output());
  if (!minimalBounds<itkVolumeType>(sgsVolume->itkImage(), SEG_BG_VALUE).areValid())
  {
    cerr << "Invalid dilate input. Failed to update trace filter." << endl;
    error = true;
  }

  auto loadedVolume = readLockVolume(loadedDilateOuptut);
  if (!minimalBounds<itkVolumeType>(loadedVolume->itkImage(), SEG_BG_VALUE).areValid())
  {
    cerr << "Invalid dilate output" << endl;
    error = true;
  }

  file.absoluteDir().remove(file.fileName());

  return error;
}
