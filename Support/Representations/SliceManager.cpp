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

#include "SliceManager.h"
#include "RepresentationUtils.h"
#include <GUI/View/RenderView.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
SliceManager::SliceManager()
: ActorManager(ViewType::VIEW_2D)
, m_plane{Plane::UNDEFINED}
{
}

//----------------------------------------------------------------------------
RepresentationManager::PipelineStatus SliceManager::pipelineStatus() const
{
  return PipelineStatus::RANGE_DEPENDENT;
}

//----------------------------------------------------------------------------
TimeRange SliceManager::readyRange() const
{
  QMap<TimeStamp, unsigned int> range;

  if(validPlane())
  {
    for(auto planePool: planePools())
    {
      auto timeRange = planePool->readyRange();

      for(auto time: timeRange)
      {
        range[time] += 1;
      }
    }
  }

  TimeRange finalRange;
  unsigned int poolsNum = planePools().size();

  for(auto time: range.keys())
  {
    if(range[time] == poolsNum)
    {
      finalRange << time;
    }
  }

  return finalRange;
}

//----------------------------------------------------------------------------
void SliceManager::setResolution(const NmVector3 &resolution)
{
  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      planePool->setResolution(resolution);
    }
  }
}

//----------------------------------------------------------------------------
void SliceManager::setPlane(Plane plane)
{
  Q_ASSERT(Plane::UNDEFINED == m_plane);

  m_plane = plane;

  if(validPlane())
  {
    for(auto planePool: planePools())
    {
      RepresentationUtils::setPlane(planePool, plane);
    }
  }
}

//----------------------------------------------------------------------------
void SliceManager::setRepresentationDepth(Nm depth)
{
  if(validPlane())
  {
    for(auto planePool: planePools())
    {
      RepresentationUtils::setSegmentationDepth(planePool, depth);
    }
  }
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr SliceManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  ViewItemAdapterPtr pickedItem = nullptr;

  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      pickedItem = planePool->pick(point, actor);

      if(pickedItem) break;
    }
  }

  return pickedItem;
}

//----------------------------------------------------------------------------
void SliceManager::addPool(RepresentationPoolSPtr pool, Plane plane)
{
  RepresentationManager::addPool(pool);

  switch(plane)
  {
    case Plane::XY:
      m_poolsXY << pool;
      break;
    case Plane::XZ:
      m_poolsXZ << pool;
      break;
    case Plane::YZ:
      m_poolsYZ << pool;
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}

//----------------------------------------------------------------------------
bool SliceManager::hasSources() const
{
  bool result = false;

  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      result |= planePool->hasSources();
    }
  }

  return result;
}

//----------------------------------------------------------------------------
void SliceManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      planePool->setCrosshair(crosshair, time);
    }
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors SliceManager::actors(TimeStamp time)
{
  RepresentationPipeline::Actors actors;

  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      auto poolActors = planePool->actors(time);

      for(auto item: poolActors.keys())
      {
        actors[item] << poolActors[item];
      }
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
void SliceManager::invalidatePreviousActors(TimeStamp time)
{
  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      planePool->invalidatePreviousActors(time);
    }
  }
}

//----------------------------------------------------------------------------
void SliceManager::connectPools()
{
  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      connect(planePool.get(), SIGNAL(actorsReady(TimeStamp)),
              this,              SLOT(emitRenderRequest(TimeStamp)));

      connect(planePool.get(), SIGNAL(actorsInvalidated()),
              this,              SLOT(invalidateRepresentations()));

      planePool->incrementObservers();
    }
  }
}

//----------------------------------------------------------------------------
void SliceManager::disconnectPools()
{
  if (validPlane())
  {
    for(auto planePool: planePools())
    {
      disconnect(planePool.get(), SIGNAL(actorsReady(TimeStamp)),
                 this,              SLOT(emitRenderRequest(TimeStamp)));
      disconnect(planePool.get(), SIGNAL(actorsInvalidated()),
                 this,              SLOT(invalidateRepresentations()));

      planePool->decrementObservers();
    }
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr SliceManager::cloneImplementation()
{
  auto clone = std::make_shared<SliceManager>();

  clone->m_name          = m_name;
  clone->m_description   = m_description;
  clone->m_plane         = m_plane;
  clone->m_showRepresentations = m_showRepresentations;

  for(auto pool: m_poolsXY)
  {
    clone->addPool(pool, Plane::XY);
  }

  for(auto pool: m_poolsXZ)
  {
    clone->addPool(pool, Plane::XZ);
  }

  for(auto pool: m_poolsYZ)
  {
    clone->addPool(pool, Plane::YZ);
  }

  return clone;
}

//----------------------------------------------------------------------------
RepresentationPoolSList SliceManager::planePools() const
{
  switch (m_plane)
  {
    case Plane::XY:
      return m_poolsXY;
    case Plane::XZ:
      return m_poolsXZ;
    case Plane::YZ:
      return m_poolsYZ;
    case Plane::UNDEFINED:
      Q_ASSERT(false);
      break;
  };

  return RepresentationPoolSList();
}

//----------------------------------------------------------------------------
bool SliceManager::validPlane() const
{
  return Plane::UNDEFINED != m_plane;
}
