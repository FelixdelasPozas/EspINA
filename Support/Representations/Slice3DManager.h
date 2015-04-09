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

#ifndef ESPINA_SLICE_3D_MANAGER_H
#define ESPINA_SLICE_3D_MANAGER_H

// ESPINA
#include <GUI/Representations/ActorManager.h>
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA {

  class Slice3DManager
  : public ActorManager
  {
    Q_OBJECT
  public:
    Slice3DManager(RepresentationPoolSPtr poolXY,
                   RepresentationPoolSPtr poolXZ,
                   RepresentationPoolSPtr poolYZ);

    virtual ~Slice3DManager();

    virtual TimeRange readyRange() const;

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

  private:
    virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp time) override;

    virtual void onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t);

    virtual RepresentationPipeline::Actors actors(TimeStamp time) override;

    virtual void invalidatePreviousActors(TimeStamp time) override;

    virtual void connectPools()      override;

    virtual void disconnectPools()   override;

    virtual void hideActors(TimeStamp t) override;

    virtual RepresentationManagerSPtr cloneImplementation();

  private slots:
    void checkRenderRequest();

  private:
    RepresentationPoolSList m_pools;
  };
}

#endif // ESPINA_SLICE_3D_MANAGER_H
