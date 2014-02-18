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

#include "SegmentationExtensionSupport.h"
#include "segmentation_testing_support.h"

using namespace std;
using namespace EspINA;
using namespace EspINA::Testing;

int segmentation_save_snapshot(int argc, char** argv)
{
  bool error = false;

  SegmentationExtensionSPtr extension{ new DummySegmentationExtension() };
  Classification classification;
  SegmentationSPtr segmentation{new Segmentation(InputSPtr())};

  segmentation->addExtension(extension);

  Snapshot snapshot = segmentation->snapshot();

  if (snapshot.size() != 2)
  {
    cerr << "Invalid number of snapshot data" << endl;
    error = true;
  }

  QString dir = "Segmentations/" + segmentation->uuid().toString() + "/";

  if (snapshot[0].first != dir + "DummySegmentationExtension.txt")
  {
    cerr << "Extension Snapshot not found" << endl;
    error = true;
  }

  if (snapshot[1].first != dir + "extensions.xml")
  {
    cerr << "Extensions file not found" << endl;
    error = true;
  }

  return error;
}
