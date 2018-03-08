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

#ifndef ESPINA_REPRESENTATION_FACTORY_H
#define ESPINA_REPRESENTATION_FACTORY_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/RepresentationManager.h>
#include <Support/Context.h>
#include "RepresentationSwitch.h"

namespace ESPINA
{
  using RepresentationGroup = QString;

  /** \brief Group of objects which are coordinated to render
   *         an specific type of related items
   *
   */
  struct Representation
  {
    RepresentationGroup                              Group;    /** Representation name.                    */
    RepresentationPoolSList                          Pools;    /** List of pools of the representation.    */
    GUI::Representations::RepresentationManagerSList Managers; /** List of managers of the representation. */
    RepresentationSwitchSList                        Switches; /** List of switches of the representation. */
    PoolSettingsSList                                Settings; /** List of settings of the representation. */
  };

  using RepresentationList = QList<Representation>;

  /** \brief Create specific representation drivers
   *
   */
  class EspinaSupport_EXPORT RepresentationFactory
  {
    public:
      /** \brief RepresentationFactory class virtual destructor.
       *
       */
      virtual ~RepresentationFactory()
      {}

      /** \brief Create a group of objects which are coordinated
       *         to display an specific type of related elements
       *
       */
      Representation createRepresentation(Support::Context &context, ViewTypeFlags supportedViews = ViewType::VIEW_2D|ViewType::VIEW_3D) const
      {
        return doCreateRepresentation(context, supportedViews);
      }

    private:
      virtual Representation doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const = 0;
  };
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_FACTORY_H
