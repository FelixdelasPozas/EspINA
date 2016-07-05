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

// Plugin
#include "CountingFrameFactories.h"
#include "CountingFrameExtension.h"

// ESPINA
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
CFStackExtensionFactory::CFStackExtensionFactory(CoreFactory *factory, CountingFrameManager* manager, SchedulerSPtr scheduler)
: StackExtensionFactory{factory}
, m_manager            {manager}
, m_scheduler          {scheduler}
{
}

//-----------------------------------------------------------------------------
StackExtensionSPtr CFStackExtensionFactory::createExtension(const StackExtension::Type      &type,
                                                            const StackExtension::InfoCache &cache,
                                                            const State                     &state) const
{
  if (type != CountingFrameExtension::TYPE)
  {
    auto what    = QObject::tr("Unable to create stack extension, extension type:  %1").arg(type);
    auto details = QObject::tr("ChannelExtensionFactoryCF::createChannelExtension() -> Unable to create stack extension, extension type:  %1").arg(type);

    throw EspinaException(what, details);
  }

  return StackExtensionSPtr{new CountingFrameExtension(m_manager, m_scheduler, m_factory, state)};
}

//-----------------------------------------------------------------------------
StackExtension::TypeList CFStackExtensionFactory::providedExtensions() const
{
  StackExtension::TypeList extensions;

  extensions << CountingFrameExtension::TYPE;

  return extensions;
}

//-----------------------------------------------------------------------------
CFSegmentationExtensionFactory::CFSegmentationExtensionFactory(CoreFactory *factory)
: SegmentationExtensionFactory{factory}
{
}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr CFSegmentationExtensionFactory::createExtension(const SegmentationExtension::Type      &type,
                                                                          const SegmentationExtension::InfoCache &cache,
                                                                          const State& state) const
{
  if (type != StereologicalInclusion::TYPE)
  {
    auto what    = QObject::tr("Unable to create segmentation extension, extension type:  %1").arg(type);
    auto details = QObject::tr("ChannelExtensionFactoryCF::createSegmentationExtension() -> Unable to create segmentation extension, extension type:  %1").arg(type);

    throw EspinaException(what, details);
  }

  return SegmentationExtensionSPtr{new StereologicalInclusion(cache)};
}

//-----------------------------------------------------------------------------
SegmentationExtension::TypeList CFSegmentationExtensionFactory::providedExtensions() const
{
  SegmentationExtension::TypeList extensions;

  extensions << StereologicalInclusion::TYPE;

  return extensions;
}
