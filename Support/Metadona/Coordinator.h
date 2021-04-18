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

#ifndef ESPINA_METADONA_PROVIDER_H
#define ESPINA_METADONA_PROVIDER_H

#include "Support/EspinaSupport_Export.h"

#include <Coordinator.h>

namespace ESPINA
{
  /** \class Coordinator
   * \brief
   */
  class EspinaSupport_EXPORT Coordinator
  : public Metadona::Coordinator
  {
    public:
      /** \brief Coordinator class virtual destructor.
       *
       */
      virtual ~Coordinator()
      {};

      /** \brief Shows the selector dialog for the next possible entries.
       * \param[in] level level of the metadata.
       * \param[in] entries list of possible entries.
       *
       */
      virtual Metadona::Id selectEntry(const Metadona::Level& level, std::vector<Metadona::Id> entries) const;

      /** \brief Creates and shows a dialog for the given entry.
       * \param[in] entry entry data.
       *
       */
      virtual void createEntry(Metadona::Entry& entry);

      /** \brief Shows a dialog to select the next metadata level.
       * \param[in] levels list of metadata levels.
       *
       */
      virtual Metadona::Level selectNextLevel(const std::vector<Metadona::Level>& levels);
  };
}

#endif // ESPINA_METADONA_PROVIDER_H
