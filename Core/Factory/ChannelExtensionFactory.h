/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_CHANNEL_EXTENSION_FACTORY_H
#define ESPINA_CHANNEL_EXTENSION_FACTORY_H

#include "Core/EspinaCore_Export.h"

#include <Core/Analysis/Extension.h>

namespace EspINA
{
  class EspinaCore_EXPORT ChannelExtensionFactory
  {
  public:
    struct Extension_Not_Provided_Exception {};

  public:
    virtual ~ChannelExtensionFactory() {}

    virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type      &type,
                                                        const ChannelExtension::InfoCache &cache = ChannelExtension::InfoCache() ,
                                                        const State                       &state = State()) const = 0;

    virtual ChannelExtensionTypeList providedExtensions() const = 0 ;
  };

  using ChannelExtensionFactoryPtr   = ChannelExtensionFactory *;
  using ChannelExtensionFactorySPtr  = std::shared_ptr<ChannelExtensionFactory>;
  using ChannelExtensionFactorySList = QList<ChannelExtensionFactorySPtr>;

}// namespace EspINA

#endif // ESPINA_CHANNEL_EXTENSION_FACTORY_H