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

#ifndef EXTENSIONS_EDGEDISTANCES_CHANNELEDGESFACTORY_H_
#define EXTENSIONS_EDGEDISTANCES_CHANNELEDGESFACTORY_H_

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  class CoreFactory;

  namespace Extensions
  {
    /** \class ChannelEdgesFactory
     * \brief Factory for the ChannelEdges stack extension.
     */
    class EspinaExtensions_EXPORT ChannelEdgesFactory
    : public Core::StackExtensionFactory
    {
      public:
        /** \brief ChannelEdgesFactory class constructor.
         * \param[in] factory pointer to CoreFactory.
         *
         */
        explicit ChannelEdgesFactory(CoreFactory *factory);

        /** \brief ChannelEdgesFactory class virtual destructor.
         *
         */
        virtual ~ChannelEdgesFactory();

        virtual Core::StackExtensionSPtr createExtension(const Core::StackExtension::Type      &type,
                                                         const Core::StackExtension::InfoCache &cache = Core::StackExtension::InfoCache() ,
                                                         const State                           &state = State()) const;

        virtual Core::StackExtension::TypeList providedExtensions() const;
    };

    using ChannelEdgesFactoryPtr  = ChannelEdgesFactory *;
    using ChannelEdgesFactorySPtr = std::shared_ptr<ChannelEdgesFactory>;

  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_EDGEDISTANCES_CHANNELEDGESFACTORY_H_
