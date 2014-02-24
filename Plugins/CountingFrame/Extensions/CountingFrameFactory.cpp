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

#include "CountingFrameFactory.h"
#include "CountingFrameExtension.h"

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameFactory::CountingFrameFactory(CountingFrameManager* manager, SchedulerSPtr scheduler)
: m_manager(manager)
, m_scheduler(scheduler)
{
}

//-----------------------------------------------------------------------------
ChannelExtensionSPtr CountingFrameFactory::createChannelExtension(const ChannelExtension::Type type, const State& state) const
{
  if (type != CountingFrameExtension::TYPE)
    throw (-1);

  return m_manager->createExtension(m_scheduler, state);
}

//-----------------------------------------------------------------------------
ChannelExtensionTypeList CountingFrameFactory::providedExtensions() const
{
  ChannelExtensionTypeList extensions;

  extensions << CountingFrameExtension::TYPE;

  return extensions;
}
