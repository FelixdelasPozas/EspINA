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
#include <GUI/Representations/Managers/PoolManager.h>
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA {

  class Slice3DManager
  : public GUI::Representations::Managers::PoolManager
  {
    Q_OBJECT
  public:
    Slice3DManager(RepresentationPoolSPtr poolXY,
                   RepresentationPoolSPtr poolXZ,
                   RepresentationPoolSPtr poolYZ,
                   ManagerFlags           flags = ManagerFlags());

    virtual TimeRange readyRangeImplementation() const override;

    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const override;

  protected:
    virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

    virtual bool acceptSceneResolutionChange( const NmVector3 &resolution) const override;

    virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

  private:
    virtual bool hasRepresentations() const override;

    virtual void updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t) override;

    virtual void changeCrosshair(const NmVector3 &crosshair, TimeStamp t) override;

    virtual void changeSceneResolution(const NmVector3 &resolution, TimeStamp t) override;

    virtual void onShow(TimeStamp t);

    virtual void onHide(TimeStamp t);

    virtual RepresentationPipeline::Actors actors(TimeStamp t) override;

    virtual void invalidatePreviousActors(TimeStamp t) override;

    virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

    void connectPools();

    void disconnectPools();

    virtual void updateSettingsImplementation(std::shared_ptr<RepresentationPool::Settings> settings, TimeStamp t) override;

  private slots:
    void checkRenderRequest();

  private:
    RepresentationPoolSList m_pools;
  };
}

#endif // ESPINA_SLICE_3D_MANAGER_H
