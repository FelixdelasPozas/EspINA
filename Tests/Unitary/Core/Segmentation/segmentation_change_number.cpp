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

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Analysis.h"

using namespace EspINA;
using namespace std;

int segmentation_change_number(int argc, char** argv)
{
  SegmentationSPtr segmentation{new Segmentation(FilterSPtr(), 0)};

  if (segmentation->number() != 0)
  {
    cerr << "Segmentation number wrong on construction!" << std::endl;
    return true;
  }

  segmentation->setNumber(1);
  if (segmentation->number() != 1)
  {
    cerr << "Segmentation number modification failed!" << std::endl;
    return true;
  }

  return false;
}