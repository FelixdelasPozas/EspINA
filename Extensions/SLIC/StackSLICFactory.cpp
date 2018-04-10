/*
 * Copyright (C) 2018, Álvaro Muñoz Fernández <golot@golot.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Extensions.h>
#include "StackSLICFactory.h"
#include "StackSLIC.h"

using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//-----------------------------------------------------------------------
StackSLICFactory::StackSLICFactory(CoreFactory *factory)
: StackExtensionFactory{factory}
{
}

//-----------------------------------------------------------------------
StackSLICFactory::~StackSLICFactory()
{
}

//-----------------------------------------------------------------------
StackExtensionSPtr StackSLICFactory::createExtension(const Core::StackExtension::Type      &type,
                                                            const Core::StackExtension::InfoCache &cache,
                                                            const State                           &state) const
{
  StackExtensionSPtr extension = nullptr;

  if(type == StackSLIC::TYPE)
  {
    extension = StackExtensionSPtr{new StackSLIC(m_factory->scheduler(), cache)};
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown extension type: %1").arg(type);
    auto details = QObject::tr("StackSLICFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//-----------------------------------------------------------------------
StackExtension::TypeList StackSLICFactory::providedExtensions() const
{
  StackExtension::TypeList list;
  list << StackSLIC::TYPE;

  return list;
}
