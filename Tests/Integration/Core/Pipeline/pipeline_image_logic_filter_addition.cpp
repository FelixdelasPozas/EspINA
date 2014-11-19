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
#include <Filters/ImageLogicFilter.h>
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::IO;

int pipeline_image_logic_filter_addition( int argc, char** argv )
{
  class TestFilterFactory
  : public FilterFactory
  {
    virtual FilterTypeList providedFilters() const
    {
      FilterTypeList list;
      list << "SGS" << "ADD";
      return list;
    }

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
    {
      FilterSPtr filter;

      if ("SGS" == type) {
        filter = make_shared<SeedGrowSegmentationFilter>(inputs, type, scheduler);
      }
      else if ("ADD" == type)
      {
        filter = make_shared<ImageLogicFilter>(inputs, type, scheduler);
      }
      filter->setDataFactory(make_shared<MarchingCubesFromFetchedVolumetricData>());

      return filter;
    }
  };

  bool error = false;

  SchedulerSPtr scheduler;

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

  auto segFilter1 = make_shared<SeedGrowSegmentationFilter>(inputs, "SGS", scheduler);
  segFilter1->update();

  auto segmentation1 = make_shared<Segmentation>(getInput(segFilter1, 0));
  segmentation1->setNumber(1);
  analysis.add(segmentation1);

  auto segFilter2 = make_shared<SeedGrowSegmentationFilter>(inputs, "SGS", scheduler);
  segFilter2->update();

  auto segmentation2 = make_shared<Segmentation>(getInput(segFilter2, 0));
  segmentation2->setNumber(2);
  analysis.add(segmentation2);

  InputSList mergeInputs;
  mergeInputs << segmentation1->asInput() << segmentation2->asInput();

  auto merge = make_shared<ImageLogicFilter>(mergeInputs, "ADD", scheduler);
  merge->setOperation(ImageLogicFilter::Operation::ADDITION);
  merge->update();

  auto segmentation3 = make_shared<Segmentation>(getInput(merge, 0));
  segmentation3->setNumber(3);

  analysis.remove(segmentation1);
  analysis.remove(segmentation2);
  analysis.add(segmentation3);

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

  if (analysis2->segmentations().size() != 1) {
    cerr << "Unexpeceted number of segmentations" << endl;
    error = true;
  }


//   auto loadedSegmentation = analysis2->segmentations().first();
//   auto loadedOuptut       = loadedSegmentation->output();
//   auto loadedFilter       = dynamic_cast<SeedGrowSegmentationFilter*>(loadedOuptut->filter());
//
//   if (!loadedFilter)
//   {
//     cerr << "Couldn't recover SGS filter" << endl;
//     error = true;
//   }
//
//   if (!loadedOuptut->hasData(VolumetricData<itkVolumeType>::TYPE))
//   {
//     cerr << "Expected Volumetric Data" << endl;
//     error = true;
//   }
//   else
//   {
//     auto volume = volumetricData(loadedOuptut);
//
//     if (!volume->isValid())
//     {
//       cerr << "Unexpeceted invalid volumetric data" << endl;
//       error = true;
//     }
//
//     if (volume->editedRegions().size() != 0)
//     {
//       cerr << "Unexpeceted number of edited regions" << endl;
//       error = true;
//     }
//
//     TemporalStorageSPtr tmpStorage(new TemporalStorage());
//     for (auto snapshot : volume->snapshot(tmpStorage, "segmentation", "1"))
//     {
//       if (snapshot.first.contains("EditedRegion"))
//       {
//         cerr << "Unexpected edited region found" << snapshot.first.toStdString() << endl;
//         error = true;
//       }
//     }
//   }
//
//   if (!loadedOuptut->hasData(MeshData::TYPE))
//   {
//     cerr << "Expected Mesh Data" << endl;
//     error = true;
//   }
//   else
//   {
//     auto mesh = meshData(loadedOuptut);
//
//     if (!mesh->mesh())
//     {
//       cerr << "Expected Mesh Data Polydata" << endl;
//       error = true;
//     }
//   }

  file.absoluteDir().remove(file.fileName());

  return error;
}
