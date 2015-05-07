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

int pipeline_keep_edited_regions_on_save_filters_without_update(int argc, char** argv)
{
  class TestFilterFactory
  : public FilterFactory
  {
    virtual FilterTypeList providedFilters() const
    {
      FilterTypeList list;
      list << "DummyChannelReader" << "SGS" << "Dilate";
      return list;
    }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
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

  CoreFactorySPtr factory{new CoreFactory()};
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

  auto sgsVolume = writeLockVolume(sgs->output(0));

  Bounds modificationBounds{0,1,0,2,0,3};

  if (!Testing_Support<itkVolumeType>::Test_Pixel_Values(sgsVolume->itkImage(modificationBounds), SEG_VOXEL_VALUE))
  {
    cerr << "Unexpeceted non seg voxel value found" << endl;
    error = true;
  }

  sgsVolume->draw(modificationBounds, SEG_BG_VALUE);

  if (!Testing_Support<itkVolumeType>::Test_Pixel_Values(sgsVolume->itkImage(modificationBounds), SEG_BG_VALUE))
  {
    cerr << "Unexpeceted non bg voxel value found" << endl;
    error = true;
  }

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
  catch (SegFile::IO_Error_Exception &e)
  {
    cerr << "Couldn't save seg file" << endl;
    error = true;
  }

  AnalysisSPtr loadedAnalysis;
  try
  {
   loadedAnalysis = SegFile::load(file, factory);
  } catch (...)
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  QFileInfo file2("analysis2.seg");
  try
  {
    SegFile::save(loadedAnalysis.get(), file2);
  }
  catch (SegFile::IO_Error_Exception &e)
  {
    cerr << "Couldn't save seg file" << endl;
    error = true;
  }

  AnalysisSPtr reloadedAnalysis;
  try
  {
   reloadedAnalysis = SegFile::load(file2, factory);
  } catch (...)
  {
    cerr << "Couldn't load seg file" << endl;
    error = true;
  }

  auto reloadedSegmentation = reloadedAnalysis->segmentations().first();
  auto reloadedDilateOuptut = reloadedSegmentation->output();
  auto reloadedDilateFilter = dynamic_cast<DilateFilter*>(reloadedDilateOuptut->filter());
  if (!reloadedDilateFilter)
  {
    cerr << "Couldn't recover Dilate filter" << endl;
    error = true;
  }

  auto reloadedSGSFilter = dynamic_cast<SeedGrowSegmentationFilter *>(reloadedDilateFilter->inputs().first()->filter().get());
  if (!reloadedSGSFilter)
  {
    cerr << "Couldn't recover SGS filter" << endl;
    error = true;
  }

  auto reloadedSGSOuptut = reloadedSGSFilter->output(0);

  if (!hasVolumetricData(reloadedSGSOuptut))
  {
    cerr << "Couldn't recover SGS output" << endl;
    error = true;
  }
  else
  {
    auto volume = readLockVolume(reloadedSGSOuptut);

    if (volume->editedRegions().isEmpty())
    {
      cerr << "Unexpeceted number of edited regions" << endl;
      error = true;
    }
  }

  reloadedDilateFilter->setRadius(2);
  reloadedDilateFilter->update();

  file.absoluteDir().remove(file .fileName());
  file.absoluteDir().remove(file2.fileName());

  return error;
}
