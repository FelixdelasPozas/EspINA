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
using namespace ESPINA::Core::Analysis;
using namespace ESPINA::Testing;

int segmentation_remove_extension(int argc, char** argv)
{
  bool error = false;

  auto filter       = std::make_shared<Testing::DummyFilter>();
  auto segmentation = std::make_shared<Segmentation>(getInput(filter, 0));
  auto extension    = std::make_shared<DummySegmentationExtension>();

  auto extensions = segmentation->extensions();
  extensions->add(extension);

  error |= (!extensions->hasExtension(extension->TYPE));
  error |= (extensions->hasExtension("NoExistingSegmentation"));

  auto sameExtension = extensions["DummySegmentationExtension"];

  extensions->remove(sameExtension);

  error |= (sameExtension != extension);
  error |= (sameExtension->type() != "DummySegmentationExtension");
  error |= (extensions->hasExtension("DummySegmentationExtension"));

  auto nonIncludedExtension = std::make_shared<DummySegmentationExtension>();

  try
  {
    extensions->remove(nonIncludedExtension);
    error |= true;
  }
  catch(Extension_Not_Found &e)
  {
    error |= false;
  }

  return error;
}

