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

// Plugin
#include "ExtensionFactory.h"
#include "AppositionSurfaceExtension.h"

// ESPINA
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr ASExtensionFactory::createExtension(const SegmentationExtension::Type      &type,
                                                              const SegmentationExtension::InfoCache &cache,
                                                              const State& state) const
{
  if (type != AppositionSurfaceExtension::TYPE)
  {
    auto what    = QObject::tr("Unable to create extension: %1").arg(type);
    auto details = QObject::tr("ASExtensionFactory::createSegmentationExtension() -> Unknown extension type: %1").arg(type);

    throw EspinaException(what, details);
  }

  return SegmentationExtensionSPtr{new AppositionSurfaceExtension(cache)};
}

//-----------------------------------------------------------------------------
SegmentationExtension::TypeList ASExtensionFactory::providedExtensions() const
{
  SegmentationExtension::TypeList extensions;

  extensions << AppositionSurfaceExtension::TYPE;

  return extensions;
}
