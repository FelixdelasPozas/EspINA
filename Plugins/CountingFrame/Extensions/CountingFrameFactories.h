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

#ifndef ESPINA_CF_COUNTING_FRAME_FACTORIES_H
#define ESPINA_CF_COUNTING_FRAME_FACTORIES_H

#include <Core/Factory/ChannelExtensionFactory.h>
#include <Core/Factory/SegmentationExtensionFactory.h>
#include <CountingFrameManager.h>

namespace EspINA {
  namespace CF {

    class ChannelExtensionFactoryCF
    : public ChannelExtensionFactory
    {
    public:
      explicit ChannelExtensionFactoryCF(CountingFrameManager *manager, SchedulerSPtr scheduler);

      virtual ChannelExtensionSPtr createChannelExtension(const ChannelExtension::Type      &type,
                                                          const ChannelExtension::InfoCache &cache = ChannelExtension::InfoCache(),
                                                          const State& state = State()) const;

      virtual ChannelExtensionTypeList providedExtensions() const;

    private:
      CountingFrameManager *m_manager;
      SchedulerSPtr         m_scheduler;
    };

    class SegmentationExtensionFactoryCF
    : public SegmentationExtensionFactory
    {
    public:
      explicit SegmentationExtensionFactoryCF();

      virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                          const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache(),
                                                          const State& state = State()) const;

      virtual SegmentationExtensionTypeList providedExtensions() const;
    };

  } // namespace CF
} // namespace EspINA

#endif // ESPINA_CF_COUNTING_FRAME_FACTORIES_H
