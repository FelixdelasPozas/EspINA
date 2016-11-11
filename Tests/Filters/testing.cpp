/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "testing.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/IO/SegFile.h>
#include <Filters/DilateFilter.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <Filters/SplitFilter.h>
#include <Filters/Utils/Stencil.h>

#include <vtkPlane.h>
#include <QString>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;
using namespace ESPINA::Filters::Utils;

//------------------------------------------------------------------------
FilterTypeList ESPINA::TestFilterFactory::providedFilters() const
{
  FilterTypeList list;

  list << "SGS" << "SPLIT" << "DILATE" << "DummyChannelReader";

  return list;
}

//------------------------------------------------------------------------
FilterSPtr ESPINA::TestFilterFactory::createFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler) const
{
  FilterSPtr filter;

  if (type == "DummyChannelReader")
  {
    filter = make_shared<DummyChannelReader>();
  }
  else if ("SGS" == type) {
    filter = make_shared<SeedGrowSegmentationFilter>(inputs, type, scheduler);
  }
  else if ("SPLIT" == type)
  {
    filter = make_shared<SplitFilter>(inputs, type, scheduler);
  }
  else if ("DILATE" == type)
  {
    filter = make_shared<DilateFilter>(inputs, type, scheduler);
  }
  else
  {
    Q_ASSERT(false);
  }

  filter->setDataFactory(make_shared<MarchingCubesFromFetchedVolumetricData>());

  return filter;
}

//------------------------------------------------------------------------
SegmentationSPtr ESPINA::gls(ChannelSPtr channel)
{
  SchedulerSPtr scheduler;

  InputSList inputs;
  inputs << channel->asInput();

  auto filter = make_shared<SeedGrowSegmentationFilter>(inputs, "SGS", scheduler);
  filter->setSeed(NmVector3{1,1,1});
  filter->update();

  auto segmentation = make_shared<Segmentation>(getInput(filter, 0));
  segmentation->setNumber(1);

  return segmentation;
}

//------------------------------------------------------------------------
SegmentationSList ESPINA::split(SegmentationSPtr segmentation)
{
  auto output = segmentation->output();
  VolumeBounds planeBounds(output->bounds(), output->spacing());

  auto origin = centroid(planeBounds);

  auto plane = vtkSmartPointer<vtkPlane>::New();
  plane->SetOrigin(origin[0], origin[1], origin[2]);
  plane->SetNormal(0, 1, 0);

  InputSList splitInputs;
  splitInputs << segmentation->asInput();

  auto split = make_shared<SplitFilter>(splitInputs, "SPLIT", SchedulerSPtr());
  split->setStencil(Stencil::fromPlane(plane, planeBounds));
  split->update();

  SegmentationSList result;

  auto segmentation2 = make_shared<Segmentation>(getInput(split, 0));
  segmentation2->setNumber(2);
  result << segmentation2;

  auto segmentation3 = make_shared<Segmentation>(getInput(split, 1));
  segmentation3->setNumber(3);
  result << segmentation3;

  return result;
}

//------------------------------------------------------------------------
bool ESPINA::dilate(SegmentationSPtr segmentation)
{
  auto beforeBounds = segmentation->bounds();

  InputSList inputs;
  inputs << segmentation->asInput();

  auto dilate = std::make_shared<DilateFilter>(inputs, "DILATE", SchedulerSPtr());
  dilate->setRadius(1);
  dilate->update();

  segmentation->changeOutput(getInput(dilate, 0));

  bool error = false;

  if ( segmentation->bounds() == beforeBounds
    || !contains(segmentation->bounds(), beforeBounds))
  {
    cerr << "Dilate bounds should be bigger than original ones" << endl;
    error = true;
  }

  if (!dynamic_cast<DilateFilter*>(segmentation->output()->filter()))
  {
    cerr << "Output filter is not a dilate filter" << endl;
    error = true;
  }

  return error;
}

//------------------------------------------------------------------------
bool ESPINA::checkSplitBounds(SegmentationSPtr source, SegmentationSPtr split1, SegmentationSPtr split2)
{
  bool error = false;

  auto bounds1 = source->bounds();
  auto bounds2 = split1->bounds();
  auto bounds3 = split2->bounds();

  for (auto i : {0, 1, 4, 5})
  {
    if ( bounds1[i] != bounds2[i]
      || bounds1[i] != bounds3[i])
    {
      std::cerr << "Incorrect bounds on dimension " << i << std::endl;
      error = true;
    }
  }

  auto halfBounds1 = (bounds1[3] + bounds1[2]) / 2;
  if ( halfBounds1 != bounds2[3]
    || halfBounds1 != bounds3[2])
  {
    std::cerr << "Incorrect bounds on split dimension " << std::endl;
    error = true;
  }

  return error;
}

//------------------------------------------------------------------------
AnalysisSPtr ESPINA::loadAnalyisis(QFileInfo file, CoreFactorySPtr factory)
{
  AnalysisSPtr analysis;

  try
  {
   analysis = IO::SegFile::load(file, factory);
  } catch (...)
  {
  }

  return analysis;
}

//------------------------------------------------------------------------
bool ESPINA::checkSegmentations(AnalysisSPtr analysis, int number)
{
  bool error = analysis->segmentations().size() != number;

  if (error)
  {
    cerr << "Unexpeceted number of segmentations" << endl;
  }

  return error;
}

//------------------------------------------------------------------------
bool ESPINA::checkValidData(SegmentationSPtr segmentation, int numVolumeEditedRegions)
{
  bool error = false;

  auto loadedOuptut = segmentation->output();

  if (!loadedOuptut->hasData(VolumetricData<itkVolumeType>::TYPE))
  {
    cerr << "Expected Volumetric Data" << endl;
    error = true;
  }
  else
  {
    auto volume = writeLockVolume(loadedOuptut);

    if (!volume->isValid())
    {
      cerr << "Unexpected invalid volumetric data" << endl;
      error = true;
    }

    if (volume->editedRegions().size() != numVolumeEditedRegions)
    {
      cerr << "Unexpected number of edited regions" << endl;
      error = true;
    }

    auto tmpStorage   = make_shared<TemporalStorage>();
    auto snapshotData = volume->snapshot(tmpStorage, QString{"segmentation"}, QString{"1"});
    for (auto snapshot : snapshotData)
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
    auto mesh = readLockMesh(loadedOuptut);

    if (!mesh->mesh())
    {
      cerr << "Expected Mesh Data Polydata" << endl;
      error = true;
    }
  }

  return error;
}

//------------------------------------------------------------------------
bool ESPINA::checkSpacing(const NmVector3& lhs, const NmVector3& rhs)
{
  bool error = lhs != rhs;

  if (error)
  {
    cerr << "Unexpected spacing " << lhs << rhs << endl;
  }
  return error;
}



