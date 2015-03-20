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
  VolumetricManager::VolumetricManager()
  : ActorManager(ViewType::VIEW_3D)
  , m_lastTime{0}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationManager::PipelineStatus VolumetricManager::pipelineStatus() const
  {
    // TODO: this is Ready, BasicRepresentationPool needs to be changed or derived.
    return PipelineStatus::RANGE_DEPENDENT;
  }

  //----------------------------------------------------------------------------
  TimeRange VolumetricManager::readyRange() const
  {
    QMap<TimeStamp, int> count;

    for (auto pool : managedPools())
    {
      for(auto timeStamp: pool->readyRange())
      {
        count[timeStamp] = count.value(timeStamp, 0) + 1;
      }
    }

    TimeRange range;
    auto poolsNum = managedPools().size();

    for (auto timeStamp : count.keys())
    {
      if (count[timeStamp] == poolsNum)
      {
        range << timeStamp;
      }
    }

    return range;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setResolution(const NmVector3 &resolution)
  {
  }

  //----------------------------------------------------------------------------
  ViewItemAdapterPtr VolumetricManager::pick(const NmVector3 &point, vtkProp *actor) const
  {
    ViewItemAdapterPtr pickedItem = nullptr;

    for (auto pool : managedPools())
    {
      auto pickedItem = pool->pick(point, actor);

      if (pickedItem) break;
    }

    return pickedItem;

  }

  //----------------------------------------------------------------------------
  bool VolumetricManager::hasSources() const
  {
    bool result = true;

    for (auto pool : managedPools())
    {
      result &= pool->hasSources();
    }

    return result;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
  {
    for(auto pool: managedPools())
    {
      pool->setCrosshair(crosshair, time);
    }
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::Actors VolumetricManager::actors(TimeStamp time)
  {
    RepresentationPipeline::Actors actors;

    for (auto pool : managedPools())
    {
      auto poolActors = pool->actors(time);

      for(auto it = poolActors.begin(); it != poolActors.end(); ++it)
      {
        actors[it.key()] << it.value();
      }
    }

    return actors;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::invalidatePreviousActors(TimeStamp time)
  {
    for (auto pool : managedPools())
    {
      pool->invalidatePreviousActors(time);
    }
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::connectPools()
  {
    for (auto pool : managedPools())
    {
      connect(pool.get(), SIGNAL(poolUpdated(TimeStamp)),
              this,       SLOT(checkRenderRequest()));
      connect(pool.get(), SIGNAL(actorsInvalidated()),
              this,       SLOT(invalidateRepresentations()));

      pool->incrementObservers();
    }
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::disconnectPools()
  {
    for (auto pool : managedPools())
    {
      disconnect(pool.get(), SIGNAL(poolUpdated(TimeStamp)),
                 this,       SLOT(checkRenderRequest()));
      disconnect(pool.get(), SIGNAL(actorsInvalidated()),
                 this,       SLOT(invalidateRepresentations()));

      pool->decrementObservers();
    }
  }

  //----------------------------------------------------------------------------
  RepresentationManagerSPtr VolumetricManager::cloneImplementation()
  {
    auto clone = std::make_shared<VolumetricManager>();
    clone->m_name          = m_name;
    clone->m_description   = m_description;
    clone->m_showRepresentations = m_showRepresentations;

    for(auto pool: managedPools())
    {
      clone->addPool(pool);
    }

    return clone;
  }

  //----------------------------------------------------------------------------
  void VolumetricManager::checkRenderRequest()
  {
    TimeStamp lastTime = std::numeric_limits<unsigned long long>::max();

    for(auto pool: managedPools())
    {
      auto poolTime = pool->lastUpdateTimeStamp();
      if(poolTime < lastTime)
      {
        lastTime = poolTime;
      }
    }

    emitRenderRequest(lastTime);
  }

} // namespace ESPINA
