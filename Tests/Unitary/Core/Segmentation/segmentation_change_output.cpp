/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Analysis.h"
#include "Core/MultiTasking/Scheduler.h"
#include "segmentation_testing_support.h"

using namespace EspINA;
using namespace EspINA::Testing;

int segmentation_change_output(int argc, char** argv)
{
  FilterSPtr filter{ new DummyFilter() };
  auto input = getInput(filter, 0);

  SegmentationSPtr segmentation{new Segmentation(input)};

  OutputSPtr oldOutput = segmentation->output();

  FilterSPtr newFilter{new DummyFilter()};

  segmentation->changeOutput(newFilter, 0);

  // TODO: real change output
  bool error = false;
  if (newFilter->output(0) != segmentation->output())
  {
    error = true;
  }

  if (oldOutput == segmentation->output())
  {
    error = true;
  }

  return error;
}
