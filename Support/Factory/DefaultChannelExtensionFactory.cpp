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

#include "DefaultChannelExtensionFactory.h"

#include <Extensions/EdgeDistances/ChannelEdges.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
DefaultChannelExtensionFactory::DefaultChannelExtensionFactory(SchedulerSPtr scheduler)
: m_scheduler(scheduler)
{
}

//-----------------------------------------------------------------------------
ChannelExtensionSPtr DefaultChannelExtensionFactory::createChannelExtension(const ChannelExtension::Type      &type,
                                                                            const ChannelExtension::InfoCache &cache,
                                                                            const State& state) const
{
  ChannelExtensionSPtr extension;

  if (ChannelEdges::TYPE == type)
  {
    extension = ChannelExtensionSPtr{new ChannelEdges(m_scheduler, cache, state)};
  }

  return extension;
}

//-----------------------------------------------------------------------------
ChannelExtensionTypeList DefaultChannelExtensionFactory::providedExtensions() const
{
  ChannelExtensionTypeList extensionTypes;

  extensionTypes << ChannelEdges::TYPE;

  return extensionTypes;
}
