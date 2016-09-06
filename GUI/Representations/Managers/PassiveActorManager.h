/*
 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_PASSIVE_ACTOR_MANAGER_H_
#define ESPINA_PASSIVE_ACTOR_MANAGER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Types.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Managers/PoolManager.h>

class vtkProp;

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      namespace Managers
      {
        /** \class PassiveActorManager
         * \brief Manager for representations that doesn't update representations on crosshair change, but on item modification.
         *
         */
        class EspinaGUI_EXPORT PassiveActorManager
        : public PoolManager
        {
          Q_OBJECT
        public:
          /** \brief PassiveActorManager class constructor
           * \param[in] pool managed pool smart pointer
           * \param[in] supportedViews flags of the views supported by this manager.
           * \param[in] flags initial flags.
           *
           */
          PassiveActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews, ManagerFlags flags = ManagerFlags());

          virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

          virtual RepresentationPoolSList pools() const override;

        private:
          virtual bool hasRepresentations() const override;

          virtual void updateFrameRepresentations(const FrameCSPtr frame) override;

          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

          virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

          virtual bool acceptInvalidationFrame(const FrameCSPtr frame) const override;

          virtual void onShow(const FrameCSPtr frame) override;

          virtual void onHide(const FrameCSPtr frame) override;

          virtual RepresentationPipeline::Actors actors(TimeStamp t) override;

          virtual void invalidatePreviousActors(TimeStamp t) override;

          /** \brief Helper method to connect the pool's signals.
           *
           */
          void connectPools();

          /** \brief Helper method to disconnect the pool's signals.
           *
           */
          void disconnectPools();

          virtual RepresentationManagerSPtr cloneImplementation() override;

        private:
          RepresentationPoolSPtr m_pool; /** managed actor's pool */
        };
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_PASSIVE_ACTOR_MANAGER_H_
