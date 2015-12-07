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

// ESPINA
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  /** \class ChannelExtensionFactory
   * \brief Extension factory for stack objects.
   *
   */
  class EspinaCore_EXPORT ChannelExtensionFactory
  {
    public:
      /** \brief ChannelExtensionFactory class virtual destructor.
       *
       */
      virtual ~ChannelExtensionFactory() {}

      /** \brief Creates a channel extension of the given type with the given state and cache object.
       * \param[in] type, channel extension type.
       * \param[in] cache, information cache object.
       * \param[in] state, state object.
       *
       */
      virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type      &type,
                                                          const ChannelExtension::InfoCache &cache = ChannelExtension::InfoCache() ,
                                                          const State                       &state = State()) const = 0;

      /** \brief Returns the list of types of channel extensions this filter can create.
       *
       */
      virtual ChannelExtensionTypeList providedExtensions() const = 0 ;
  };

  using ChannelExtensionFactoryPtr   = ChannelExtensionFactory *;
  using ChannelExtensionFactorySPtr  = std::shared_ptr<ChannelExtensionFactory>;
  using ChannelExtensionFactorySList = QList<ChannelExtensionFactorySPtr>;

}// namespace ESPINA

#endif // ESPINA_CHANNEL_EXTENSION_FACTORY_H
