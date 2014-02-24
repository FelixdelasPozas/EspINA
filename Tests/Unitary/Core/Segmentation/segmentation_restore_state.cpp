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
#include "Core/MultiTasking/Scheduler.h"
#include "segmentation_testing_support.h"

using namespace std;
using namespace EspINA;
using namespace EspINA::Testing;

int segmentation_restore_state(int argc, char** argv)
{
  bool error = false;

  SegmentationSPtr segmentation{new Segmentation(InputSPtr())};

  State forgedState = QString("Number=2;Users=FakeUser1,FakeUser2;Category=Prueba;Alias=HDP4");

  segmentation->restoreState(forgedState);

  if (segmentation->number() != 2)
  {
    cerr << "Unexpected restored number" << endl;
    error = true;
  }

  if (!(segmentation->users().contains("FakeUser1") && segmentation->users().contains("FakeUser2")))
  {
    cerr << "Unexpected user" << endl;
    error = true;
  }

  if (segmentation->alias() != "HDP4")
  {
    cerr << "Unexpected restored alias" << endl;
    error = true;
  }

  return error;
}
