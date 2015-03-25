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

#ifndef ESPINA_READY_ACTOR_MANAGER_H_
#define ESPINA_READY_REPRESENTATION_MANAGER_H_

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/RangedActorManager.h>

class vtkProp;

namespace ESPINA
{
  class ReadyActorManager
  : public RepresentationManager
  {
      Q_OBJECT
    public:
      /** \brief Volumetric Manager class constructor.
       * \param[in] pool managed pool smart pointer.
       *
       */
      ReadyActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews);

      /** \brief VolumetricManager class virtual destructor.
       *
       */
      virtual ~ReadyActorManager()
      {};

      virtual void display(TimeStamp time) override;

      virtual PipelineStatus pipelineStatus() const;

      virtual TimeRange readyRange() const;

      virtual void setResolution(const NmVector3 &resolution);

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    private:
      /** \brief Displays/Replaces the current actors in the view.
       *
       */
      void displayActors();

      /** \brief Removes all actors from the view.
       *
       */
      void removeActors();

      /** \brief Returns the pool's last valid Timestamp.
       *
       */
      TimeStamp lastValidTimeStamp() const;

      virtual void onShow() override;

      virtual void onHide() override;

      virtual bool hasSources() const;

      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) override;

      virtual void connectPools();

      virtual void disconnectPools();

      virtual RepresentationManagerSPtr cloneImplementation();

    private:
      RepresentationPoolSPtr         m_pool;
      bool                           m_init;
      RepresentationPipeline::Actors m_actors;
  };

} // namespace ESPINA

#endif // ESPINA_READY_REPRESENTATION_MANAGER_H_
