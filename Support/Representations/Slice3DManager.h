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

#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA {

  class Slice3DManager
  : public RepresentationManager
  {
    Q_OBJECT
  public:
    Slice3DManager(RepresentationPoolSPtr xy,
                   RepresentationPoolSPtr xz,
                   RepresentationPoolSPtr yz);

    virtual ~Slice3DManager();

    virtual bool isReady() const;

    virtual void onCrosshairChanged(NmVector3 crosshair);

  private:
    virtual RepresentationPipelineSList pipelines();

    virtual void updatePipelines();

    virtual void notifyPoolUsed();

    virtual void notifyPoolNotUsed();

    virtual RepresentationManagerSPtr cloneImplementation();

  private slots:
    void onPoolReady();

  private:
    RepresentationPoolSList m_pools;
  };
}

#endif // ESPINA_SLICE_3D_MANAGER_H
