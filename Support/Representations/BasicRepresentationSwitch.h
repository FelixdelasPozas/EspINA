/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_BASIC_REPRESENTATION_SWITCH_H
#define ESPINA_BASIC_REPRESENTATION_SWITCH_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Support/Representations/RepresentationSwitch.h>
#include <GUI/Representations/RepresentationManager.h>

namespace ESPINA
{
  class EspinaSupport_EXPORT BasicRepresentationSwitch
  : public RepresentationSwitch
  {
    public:
      /** \brief BasicRepresentationSwitch class constructor.
       * \param[in] id Switch id.
       * \param[in] manager manager that will be enabled/disabled & modified by the switch.
       * \param[in] supportedViews flags of the views supported by the switch.
       * \param[in] context application context.
       *
       */
      explicit BasicRepresentationSwitch(const QString &id,
                                         GUI::Representations::RepresentationManagerSPtr manager,
                                         ViewTypeFlags supportedViews,
                                         Support::Context &context);

      /** \brief BasicRepresentationSwitch class virtual destructor.
       *
       */
      virtual ~BasicRepresentationSwitch()
      {};

      virtual ViewTypeFlags supportedViews() override;

      virtual void showRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame) override;

    protected:
      GUI::Representations::RepresentationManagerSPtr m_manager;

    private:
      ViewTypeFlags m_flags;
  };
}

#endif // ESPINA_BASIC_REPRESENTATION_SWITCH_H
