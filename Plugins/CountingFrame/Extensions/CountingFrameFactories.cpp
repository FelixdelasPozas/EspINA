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
using namespace ESPINA::Core::Utils;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
ChannelExtensionFactoryCF::ChannelExtensionFactoryCF(CountingFrameManager* manager, SchedulerSPtr scheduler)
: m_manager(manager)
, m_scheduler(scheduler)
{
}

//-----------------------------------------------------------------------------
ChannelExtensionSPtr ChannelExtensionFactoryCF::createChannelExtension(const ChannelExtension::Type      &type,
                                                                       const ChannelExtension::InfoCache &cache,
                                                                       const State& state) const
{
  if (type != CountingFrameExtension::TYPE)
  {
    auto what    = QObject::tr("Unable to create stack extension, extension type:  %1").arg(type);
    auto details = QObject::tr("ChannelExtensionFactoryCF::createChannelExtension() -> Unable to create stack extension, extension type:  %1").arg(type);

    throw EspinaException(what, details);
  }

  return m_manager->createExtension(m_scheduler, state);
}

//-----------------------------------------------------------------------------
ChannelExtensionTypeList ChannelExtensionFactoryCF::providedExtensions() const
{
  ChannelExtensionTypeList extensions;

  extensions << CountingFrameExtension::TYPE;

  return extensions;
}

//-----------------------------------------------------------------------------
SegmentationExtensionFactoryCF::SegmentationExtensionFactoryCF()
{

}

//-----------------------------------------------------------------------------
SegmentationExtensionSPtr SegmentationExtensionFactoryCF::createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                                      const SegmentationExtension::InfoCache &cache,
                                                                                      const State& state) const
{
  if (type != StereologicalInclusion::TYPE)
  {
    auto what    = QObject::tr("Unable to create segmentation extension, extension type:  %1").arg(type);
    auto details = QObject::tr("ChannelExtensionFactoryCF::createSegmentationExtension() -> Unable to create segmentation extension, extension type:  %1").arg(type);

    throw EspinaException(what, details);
  }

  return std::make_shared<StereologicalInclusion>(cache);
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList SegmentationExtensionFactoryCF::providedExtensions() const
{
  SegmentationExtensionTypeList extensions;

  extensions << StereologicalInclusion::TYPE;

  return extensions;
}
