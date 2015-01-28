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

#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  class SliceManager
  : public RepresentationManager
  , public RepresentationManager2D
  {
  public:
    SliceManager(RepresentationPoolSPtr xy,
                 RepresentationPoolSPtr xz,
                 RepresentationPoolSPtr yz);

    virtual bool isReady() const;

    virtual void onCrosshairChanged(NmVector3 crosshair);

    virtual void setPlane(Plane plane);

  private:
    virtual RepresentationPipelineSList pipelines();

    virtual void updatePipelines();

    virtual void notifyPoolUsed();

    virtual void notifyPoolNotUsed();

    virtual RepresentationManagerSPtr cloneImplementation();

    RepresentationPoolSPtr planePool() const;

  private:
    bool validPlane() const;

    RepresentationPoolSPtr m_xy, m_xz, m_yz;

    Plane m_plane;
  };
}

#endif // ESPINA_SLICE_MANAGER_H
