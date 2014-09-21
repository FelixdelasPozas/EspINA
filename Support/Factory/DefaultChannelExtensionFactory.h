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

#ifndef ESPINA_DEFAULT_CHANNEL_EXTENSION_FACTORY_H
#define ESPINA_DEFAULT_CHANNEL_EXTENSION_FACTORY_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Factory/ChannelExtensionFactory.h>

namespace ESPINA {

  class EspinaSupport_EXPORT DefaultChannelExtensionFactory
  : public ChannelExtensionFactory
  {
  public:
  	/** brief DefaultChannelExtensionFactor class constructor.
  	 * \param[in] scheduler, scheduler smart pointer.
  	 *
  	 */
    explicit DefaultChannelExtensionFactory(SchedulerSPtr scheduler);

    /** brief Implements ChannelExtensionFactory::createChannelExtension().
     *
     */
    virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type      &type,
                                                        const ChannelExtension::InfoCache &cache = ChannelExtension::InfoCache(),
                                                        const State& state = State()) const;

    /** brief Implements ChannelExtensionFactory::providedExtensions().
     *
     */
    virtual ChannelExtensionTypeList providedExtensions() const;

  private:
    SchedulerSPtr m_scheduler;
  };
} // namespace ESPINA

#endif // ESPINA_DEFAULT_CHANNEL_EXTENSION_FACTORY_H
