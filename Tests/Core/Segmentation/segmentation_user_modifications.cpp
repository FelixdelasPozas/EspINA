/*
    
    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "segmentation_testing_support.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int segmentation_user_modifications(int argc, char** argv)
{
  FilterSPtr filter{new Testing::DummyFilter()};

  SegmentationSPtr segmentation(new Segmentation(getInput(filter, 0)));

  QStringList oldList = segmentation->users();
  QString newUser{"Prueba"};

  segmentation->modifiedByUser(newUser);
  QStringList newList = segmentation->users();

  if (oldList != QStringList())
  {
    cerr << "List user on constructor is not empty!" << std::endl;
    return true;
  }

  if (newList.count() != 1)
  {
    cerr << "Modified user list count is wrong!" << std::endl;
    return true;
  }

  if (!(newList.contains(newUser)))
  {
    cerr << "Modified user list doesn't contains modifications!" << std::endl;
    return true;
  }

  return false;
}
