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

#include "CountingFramePlugin_Export.h"

//Plugin
#include <CountingFrameManager.h>

// ESPINA
#include <Core/Factory/ExtensionFactory.h>

namespace ESPINA
{
  namespace CF
  {
    /** \class CFStackExtensionFactory
     * \brief Factory for counting frame extensions.
     *
     */
    class CountingFramePlugin_EXPORT CFStackExtensionFactory
    : public Core::StackExtensionFactory
    {
      public:
        /** \brief CFStackExtensionFactory class constructor.
         * \param[in] manager plugin's counting frame manager.
         * \param[in] scheduler application task scheduler.
         * \param[in] factory model factory.
         *
         */
        explicit CFStackExtensionFactory(CoreFactory *factory, CountingFrameManager *manager, SchedulerSPtr scheduler);

        virtual Core::StackExtensionSPtr createExtension(const Core::StackExtension::Type      &type,
                                                         const Core::StackExtension::InfoCache &cache = Core::StackExtension::InfoCache(),
                                                         const State                           &state = State()) const;

        virtual Core::StackExtension::TypeList providedExtensions() const;

      private:
        CountingFrameManager *m_manager;   /** plugin's counting frame manager. */
        SchedulerSPtr         m_scheduler; /** application's task scheduler.    */
    };

    /** \class CFSegmentationExtensionFactory
     * \brief Factory for stereological inclusion extensions.
     *
     */
    class CountingFramePlugin_EXPORT CFSegmentationExtensionFactory
    : public Core::SegmentationExtensionFactory
    {
      public:
        /** \brief CFSegmentationExtensionFactory class constructor.
         *
         */
        explicit CFSegmentationExtensionFactory(CoreFactory *factory);

        virtual Core::SegmentationExtensionSPtr createExtension(const Core::SegmentationExtension::Type      &type,
                                                                const Core::SegmentationExtension::InfoCache &cache = Core::SegmentationExtension::InfoCache(),
                                                                const State                                  &state = State()) const;

        virtual Core::SegmentationExtension::TypeList providedExtensions() const;
    };

  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_CF_COUNTING_FRAME_FACTORIES_H
