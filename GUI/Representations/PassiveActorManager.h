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

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/PoolManager.h>

class vtkProp;

namespace ESPINA
{
  class PassiveActorManager
  : public PoolManager
  {
      Q_OBJECT
    public:
      /** \brief Passive Actor Manager class constructor
       * \param[in] pool managed pool smart pointer
       * \param[in] supportedViews flags of the views supported by this manager.
       * \param[in] flags initial flags.
       *
       */
      PassiveActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews, ManagerFlags flags = ManagerFlags());

      virtual TimeRange readyRangeImplementation() const;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    private:
      virtual bool hasRepresentations() const;

      virtual void updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t);

      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

      virtual void onShow(TimeStamp t);

      virtual void onHide(TimeStamp t);

      virtual RepresentationPipeline::Actors actors(TimeStamp t);

      virtual void invalidatePreviousActors(TimeStamp t);

      void connectPools();

      void disconnectPools();

      virtual void displayRepresentations(TimeStamp t) override;

      virtual RepresentationManagerSPtr cloneImplementation();

      QSet<vtkProp *> toSet(const RepresentationPipeline::ActorList &list) const;

    private:
      RepresentationPoolSPtr m_pool;
  };

} // namespace ESPINA

#endif // ESPINA_PASSIVE_ACTOR_MANAGER_H_
