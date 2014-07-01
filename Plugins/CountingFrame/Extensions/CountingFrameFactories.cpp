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

#include "CountingFrameFactories.h"
#include "CountingFrameExtension.h"

using namespace EspINA;
using namespace EspINA::CF;

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
    throw Extension_Not_Provided_Exception();

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
    throw Extension_Not_Provided_Exception();

  return SegmentationExtensionSPtr{new StereologicalInclusion(cache)};
}

//-----------------------------------------------------------------------------
SegmentationExtensionTypeList SegmentationExtensionFactoryCF::providedExtensions() const
{
  SegmentationExtensionTypeList extensions;

  extensions << StereologicalInclusion::TYPE;

  return extensions;
}
