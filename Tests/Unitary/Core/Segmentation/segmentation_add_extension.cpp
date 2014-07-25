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

int segmentation_add_extension(int argc, char** argv)
{
  bool error = false;

  Classification classification;

  FilterSPtr filter{new Testing::DummyFilter()};

  SegmentationSPtr segmentation(new Segmentation(getInput(filter, 0)));

  SegmentationExtensionSPtr extension{ new DummySegmentationExtension() };
  segmentation->addExtension(extension);

  error |= (!segmentation->hasExtension("DummySegmentationExtension"));
  error |= (segmentation->hasExtension("NoExistingSegmentation"));

  using Tag = SegmentationExtension::InfoTag;
  SegmentationExtension::InfoTagList list;
  list << Tag("Tag1") << Tag("Tag2");

  error |= (segmentation->extension("DummySegmentationExtension") != extension);
  error |= (segmentation->informationTags() != list);
  error |= (segmentation->information(Tag("Tag1")) != QVariant("prueba1"));
  error |= (segmentation->information(Tag("Tag2")) != QVariant("prueba2"));

  return error;
}
