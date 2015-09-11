/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "testing.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/DilateFilter.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <Filters/SplitFilter.h>
#include <Filters/Utils/Stencil.h>

#include <vtkPlane.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Filters::Utils;

//------------------------------------------------------------------------
SegmentationSList ESPINA::gls_split(ChannelSPtr channel)
{
  SegmentationSList result;

  SchedulerSPtr scheduler;

  InputSList inputs;
  inputs << channel->asInput();

  auto segFilter1 = make_shared<SeedGrowSegmentationFilter>(inputs, "SGS", scheduler);
  segFilter1->update();

  auto segmentation1 = make_shared<Segmentation>(getInput(segFilter1, 0));
  segmentation1->setNumber(1);
  result << segmentation1;

  auto output = segmentation1->output();
  VolumeBounds planeBounds(output->bounds(), output->spacing());

  auto origin = centroid(planeBounds);

  auto plane = vtkSmartPointer<vtkPlane>::New();
  plane->SetOrigin(origin[0], origin[1], origin[2]);
  plane->SetNormal(0, 1, 0);

  InputSList splitInputs;
  splitInputs << segmentation1->asInput();

  auto split = make_shared<SplitFilter>(splitInputs, "SPLIT", scheduler);
  split->setStencil(Stencil::fromPlane(plane, planeBounds));
  split->update();

  auto segmentation2 = make_shared<Segmentation>(getInput(split, 0));
  segmentation2->setNumber(2);
  result << segmentation2;

  auto segmentation3 = make_shared<Segmentation>(getInput(split, 1));
  segmentation3->setNumber(3);
  result << segmentation3;

  return result;
}

//------------------------------------------------------------------------
FilterTypeList ESPINA::TestFilterFactory::providedFilters() const
{
  FilterTypeList list;

  list << "SGS" << "SPLIT" << "DILATE";

  return list;
}

//------------------------------------------------------------------------
FilterSPtr ESPINA::TestFilterFactory::createFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler) const
{
  FilterSPtr filter;

  if ("SGS" == type) {
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

  filter->setDataFactory(make_shared<MarchingCubesFromFetchedVolumetricData>());

  return filter;
}