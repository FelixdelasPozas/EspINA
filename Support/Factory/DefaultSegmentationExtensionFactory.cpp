/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "DefaultSegmentationExtensionFactory.h"
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <Extensions/Issues/Issues.h>
#include <Extensions/Issues/SegmentationIssues.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr DefaultSegmentationExtensionFactory::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                                           const SegmentationExtension::InfoCache &cache,
                                                                                           const State& state) const
{
  SegmentationExtensionSPtr extension;

  if (EdgeDistance::TYPE == type)
  {
    extension = std::make_shared<EdgeDistance>(cache, state);
  }
  else if (MorphologicalInformation::TYPE == type)
  {
    extension = std::make_shared<MorphologicalInformation>(cache, state);
  }
  else if (SegmentationTags::TYPE == type)
  {
    extension = std::make_shared<SegmentationTags>(cache);
  }
  else if (SegmentationNotes::TYPE == type)
  {
    extension = std::make_shared<SegmentationNotes>(cache);
  }
  else if (SegmentationIssues::TYPE == type)
  {
    extension = std::make_shared<SegmentationIssues>(cache);
  }

  return extension;
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList DefaultSegmentationExtensionFactory::providedExtensions() const
{
  SegmentationExtensionTypeList extensionTypes;

  extensionTypes << EdgeDistance::TYPE;
  extensionTypes << MorphologicalInformation::TYPE;
  extensionTypes << SegmentationTags::TYPE;
  extensionTypes << SegmentationNotes::TYPE;
  extensionTypes << SegmentationIssues::TYPE;

  return extensionTypes;
}
