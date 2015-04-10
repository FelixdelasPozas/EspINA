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
#include <Core/Utils/NmVector3.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/ActorManager.h>

class vtkProp;

namespace ESPINA
{
  class PassiveActorManager
  : public ActorManager
  {
      Q_OBJECT
    public:
      /** \brief Volumetric Manager class constructor.
       * \param[in] pool managed pool smart pointer.
       *
       */
      PassiveActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews);

      /** \brief VolumetricManager class virtual destructor.
       *
       */
      virtual ~PassiveActorManager()
      {};

      virtual TimeRange readyRange() const;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    private:
      virtual RepresentationPipeline::Actors actors(TimeStamp t);

      virtual void invalidatePreviousActors(TimeStamp t);

      virtual void connectPools();

      virtual void disconnectPools();

      virtual void displayActors(const TimeStamp t);

      virtual void showActors(TimeStamp t) override;

      virtual void hideActors(TimeStamp t) override;

      virtual RepresentationManagerSPtr cloneImplementation();

      QSet<vtkProp *> toSet(const RepresentationPipeline::ActorList &list) const;

    private:
      RepresentationPoolSPtr m_pool;
  };

} // namespace ESPINA

#endif // ESPINA_PASSIVE_ACTOR_MANAGER_H_
