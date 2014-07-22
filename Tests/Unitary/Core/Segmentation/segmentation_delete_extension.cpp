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

#include "SegmentationExtensionSupport.h"
#include "segmentation_testing_support.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int segmentation_delete_extension(int argc, char** argv)
{
  bool error = false;

  SegmentationExtensionSPtr extension{ new DummySegmentationExtension() };
  Classification classification;
  FilterSPtr filter{new Testing::DummyFilter()};

  SegmentationSPtr segmentation(new Segmentation(getInput(filter, 0)));

  segmentation->addExtension(extension);

  error |= (!segmentation->hasExtension("DummySegmentationExtension"));
  error |= (segmentation->hasExtension("NoExistingSegmentation"));

  SegmentationExtensionSPtr otherExtension = segmentation->extension("DummySegmentationExtension");

  segmentation->deleteExtension(otherExtension);

  error |= (otherExtension != extension);
  error |= (otherExtension->type() != "DummySegmentationExtension");
  error |= (segmentation->hasExtension("DummySegmentationExtension"));

  SegmentationExtensionSPtr nonIncludedExtension{ new DummySegmentationExtension() };

  try
  {
    segmentation->deleteExtension(nonIncludedExtension);
    error |= true;
  }
  catch(SegmentationExtension::Extension_Not_Found &e)
  {
    error |= false;
  }

  return error;
}

