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

#ifndef ESPINA_VOLUMETRIC_REPRESENTATION_MANAGER_H_
#define ESPINA_VOLUMETRIC_REPRESENTATION_MANAGER_H_

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>
#include <GUI/Representations/ActorManager.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Model/ViewItemAdapter.h>

class vtkProp;

namespace ESPINA
{
  class VolumetricManager
  : public ActorManager
  {
    public:
      /** \brief Volumetric Manager class constructor.
       * \param[in] pool managed pool smart pointer.
       *
       */
      VolumetricManager(RepresentationPoolSPtr pool);

      /** \brief VolumetricManager class virtual destructor.
       *
       */
      virtual ~VolumetricManager()
      {};

      virtual PipelineStatus pipelineStatus() const;

      virtual TimeRange readyRange() const;

      virtual void setResolution(const NmVector3 &resolution);

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    private:
      virtual bool hasSources() const override;

      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) override;

      virtual RepresentationPipeline::Actors actors(TimeStamp time) override;

      virtual void invalidatePreviousActors(TimeStamp time) override;

      virtual void connectPools() override;

      virtual void disconnectPools() override;

      virtual RepresentationManagerSPtr cloneImplementation();

    private:
      RepresentationPoolSPtr m_pool;
  };

} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_REPRESENTATION_MANAGER_H_
