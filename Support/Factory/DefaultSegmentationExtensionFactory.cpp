/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#include "DefaultSegmentationExtensionFactory.h"

#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <Extensions/Tags/SegmentationTags.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr DefaultSegmentationExtensionFactory::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                                           const SegmentationExtension::InfoCache &cache,
                                                                                           const State& state) const
{
  SegmentationExtensionSPtr extension;

  if (EdgeDistance::TYPE == type)
  {
    extension = SegmentationExtensionSPtr{new EdgeDistance(/*cache, state*/)};
  }
  else if (MorphologicalInformation::TYPE == type)
  {
    extension = SegmentationExtensionSPtr{new MorphologicalInformation(/*cache, state*/)};
  }
  else if (SegmentationTags::TYPE == type)
  {
    extension = SegmentationExtensionSPtr{new SegmentationTags(cache)};
  }

  return extension;
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList DefaultSegmentationExtensionFactory::providedExtensions() const
{
  SegmentationExtensionTypeList extensionTypes;

  extensionTypes << EdgeDistance::TYPE;
  extensionTypes << MorphologicalInformation::TYPE;

  return extensionTypes;
}
