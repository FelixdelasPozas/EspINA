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

#include <Extensions/EdgeDistances/AdaptiveEdges.h>

using namespace EspINA;

EspINA::ChannelExtensionTypeList DefaultChannelExtensionFactory::providedExtensions() const
{
  ChannelExtensionTypeList extensionTypes;

  extensionTypes << AdaptiveEdges::TYPE;

  return extensionTypes;
}

EspINA::ChannelExtensionSPtr DefaultChannelExtensionFactory::createChannelExtension(const ChannelExtension::Type type) const
{
  ChannelExtensionSPtr extension;

  if (AdaptiveEdges::TYPE == type)
  {
    extension = ChannelExtensionSPtr{new AdaptiveEdges()};
  }

  return extension;
}
