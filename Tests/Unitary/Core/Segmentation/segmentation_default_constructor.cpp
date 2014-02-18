/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include <Core/MultiTasking/Scheduler.h>
#include "Core/EspinaTypes.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Output.h"
#include "segmentation_testing_support.h"

using namespace std;
using namespace EspINA;
using namespace EspINA::Testing;

int segmentation_default_constructor(int argc, char** argv)
{
  FilterSPtr filter{new DummyFilter()};
  auto input = getInput(filter, 0);
  SegmentationSPtr segmentation{new Segmentation(input)};

  if (segmentation->number() != 0)
  {
    cerr << "Segmentation number is wrong!" << std::endl;
    return true;
  }

  if (segmentation->category() != nullptr)
  {
    cerr << "Segmentation category is wrong!" << std::endl;
    return true;
  }

  if (segmentation->users() != QList<QString>())
  {
    cerr << "Segmentation user list is wrong!" << std::endl;
    return true;
  }

  if (segmentation->output()->id() != 0)
  {
    cerr << "Segmentation output id is wrong!" << std::endl;
    return true;
  }

  if (segmentation->output()->filter() != filter.get())
  {
    cerr << "Segmentation filter is wrong!" << std::endl;
    return true;
  }

  return false;
}
