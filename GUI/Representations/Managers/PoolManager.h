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

#ifndef ESPINA_POOL_MANAGER_H
#define ESPINA_POOL_MANAGER_H

#include "GUI/EspinaGUI_Export.h"
#include <GUI/Representations/RepresentationManager.h>

// ESPINA
#include <Core/Types.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Representations/RepresentationPipeline.h>

// Qt
#include <QString>
#include <QIcon>

namespace ESPINA
{
  class RenderView;

  namespace GUI
  {
    namespace Representations
    {
      namespace Managers
      {
        class EspinaGUI_EXPORT PoolManager
        : public RepresentationManager
        {
          Q_OBJECT
        public:
          /** \brief PoolManager class virtual destructor.
           *
           */
          virtual ~PoolManager()
          {}

        protected:
          /** \brief PoolManager class constructor.
           * \param[in] supportedViews flags of the views supported by the manager.
           * \param[in] flags manager flags values.
           *
           */
          explicit PoolManager(ViewTypeFlags supportedViews, ManagerFlags flags);

        protected slots:
          /** \brief Invalidates frames on pool's actors invalidation.
           * \param[in] frame invalidation frame.
           *
           */
          void onActorsInvalidated(const GUI::Representations::FrameCSPtr frame);

        private:
          virtual void displayRepresentations(const FrameCSPtr frame) override;

          virtual void hideRepresentations(const FrameCSPtr frame) override;

          /** \brief Returns the list of actors corresponding to the given time.
           * \param[in] t timestamp.
           *
           */
          virtual RepresentationPipeline::Actors actors(TimeStamp t) = 0;

          /** \brief Invalidates all the actors previous to the given time.
           * \param[in] t timestamp.
           *
           */
          virtual void invalidatePreviousActors(TimeStamp time) = 0;

        protected:
          RepresentationPipeline::ActorList m_viewActors; /** actors being rendered by its view. */
        };
      }
    } // namespace Representations
  }
}// namespace ESPINA

#endif // ESPINA_POOL_MANAGER_H
