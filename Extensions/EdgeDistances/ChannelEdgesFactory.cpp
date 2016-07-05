/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Extensions.h>
#include "ChannelEdgesFactory.h"
#include "ChannelEdges.h"

using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//-----------------------------------------------------------------------
ChannelEdgesFactory::ChannelEdgesFactory(CoreFactory *factory)
: StackExtensionFactory{factory}
{
}

//-----------------------------------------------------------------------
ChannelEdgesFactory::~ChannelEdgesFactory()
{
}

//-----------------------------------------------------------------------
StackExtensionSPtr ChannelEdgesFactory::createExtension(const Core::StackExtension::Type      &type,
                                                        const Core::StackExtension::InfoCache &cache,
                                                        const State                           &state) const
{
  StackExtensionSPtr extension = nullptr;

  if(type == ChannelEdges::TYPE)
  {
    extension = StackExtensionSPtr{new ChannelEdges(m_factory->scheduler(), cache, state)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("ChannelEdgesFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//-----------------------------------------------------------------------
StackExtension::TypeList ChannelEdgesFactory::providedExtensions() const
{
  StackExtension::TypeList list;
  list << ChannelEdges::TYPE;

  return list;
}
