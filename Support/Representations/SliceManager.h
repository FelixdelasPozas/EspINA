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

#ifndef ESPINA_SLICE_MANAGER_H
#define ESPINA_SLICE_MANAGER_H

#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/ActorManager.h>

namespace ESPINA
{
  class SliceManager
  : public ActorManager
  , public RepresentationManager2D
  {
  public:
    SliceManager();

    virtual PipelineStatus pipelineStatus() const;

    virtual TimeRange readyRange() const;

    virtual void setResolution(const NmVector3 &resolution);

    virtual void setPlane(Plane plane);

    virtual void setRepresentationDepth(Nm depth);

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

    /** \brief Adds a pool to the managed pools list of the specified plane.
     *
     */
    void addPool(RepresentationPoolSPtr pool, Plane plane);

  private:
    virtual bool hasSources() const override;

    virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) override;

    virtual RepresentationPipeline::Actors actors(TimeStamp time) override;

    virtual void invalidatePreviousActors(TimeStamp time) override;

    virtual void connectPools()      override;

    virtual void disconnectPools()   override;

    virtual RepresentationManagerSPtr cloneImplementation();

    RepresentationPoolSList planePools() const;

    bool validPlane() const;

  private:
    Plane m_plane;
    RepresentationPoolSList m_poolsXY;
    RepresentationPoolSList m_poolsXZ;
    RepresentationPoolSList m_poolsYZ;
  };
}

#endif // ESPINA_SLICE_MANAGER_H
