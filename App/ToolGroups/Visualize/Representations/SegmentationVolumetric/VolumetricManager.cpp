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

// ESPINA
#include <ToolGroups/Visualize/Representations/SegmentationVolumetric/VolumetricManager.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  VolumetricManager::VolumetricManager(RepresentationPoolSPtr pool)
  : ActorManager{ViewType::VIEW_3D}
  , m_pool      {pool}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationManager::PipelineStatus VolumetricManager::pipelineStatus() const
  {
    return PipelineStatus::READY;
  }

  //----------------------------------------------------------------------------
  TimeRange VolumetricManager::readyRange() const
  {
    Q_ASSERT(false);
    return TimeRange();
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setResolution(const NmVector3 &resolution)
  {
  }

  //----------------------------------------------------------------------------
  ViewItemAdapterPtr VolumetricManager::pick(const NmVector3 &point, vtkProp *actor) const
  {
    return m_pool->pick(point, actor);
  }

  //----------------------------------------------------------------------------
  bool VolumetricManager::hasSources() const
  {
    return m_pool->hasSources();
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
  {
    qDebug() << "volumetric manager set crosshair";
    m_pool->setCrosshair(crosshair, time);
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::Actors VolumetricManager::actors(TimeStamp time)
  {
    return m_pool->actors(time);
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::invalidatePreviousActors(TimeStamp time)
  {
    m_pool->invalidatePreviousActors(time);
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::connectPools()
  {
    connect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
            this,         SLOT(emitRenderRequest(TimeStamp)));

    connect(m_pool.get(), SIGNAL(actorsInvalidated()),
            this,         SLOT(invalidateRepresentations()));

    m_pool->incrementObservers();
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::disconnectPools()
  {
    disconnect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
               this,         SLOT(emitRenderRequest(TimeStamp)));

    disconnect(m_pool.get(), SIGNAL(actorsInvalidated()),
              this,         SLOT(invalidateRepresentations()));

    m_pool->decrementObservers();
  }

  //----------------------------------------------------------------------------
  RepresentationManagerSPtr VolumetricManager::cloneImplementation()
  {
    auto clone = std::make_shared<VolumetricManager>(m_pool);
    clone->m_name          = m_name;
    clone->m_description   = m_description;
    clone->m_showRepresentations = m_showRepresentations;

    return clone;
  }

} // namespace ESPINA
