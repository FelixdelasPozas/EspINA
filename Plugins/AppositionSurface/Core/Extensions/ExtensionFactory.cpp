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

// ESPINA
#include "ExtensionFactory.h"
#include "AppositionSurfaceExtension.h"

//-----------------------------------------------------------------------------
ASExtensionFactory::ASExtensionFactory()
{
}

//-----------------------------------------------------------------------------
ASExtensionFactory::~ASExtensionFactory()
{
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr ASExtensionFactory::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                          const SegmentationExtension::InfoCache &cache,
                                                                          const State& state) const
{
  if (type != AppositionSurfaceExtension::TYPE)
    throw Extension_Not_Provided_Exception();

  return SegmentationExtensionSPtr{new AppositionSurfaceExtension(cache)};
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList ASExtensionFactory::providedExtensions() const
{
  SegmentationExtensionTypeList extensions;

  extensions << AppositionSurfaceExtension::TYPE;

  return extensions;
}
