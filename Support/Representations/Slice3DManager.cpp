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

#include "Slice3DManager.h"
#include <vtkProp.h>

using namespace ESPINA;
using namespace std;

//----------------------------------------------------------------------------
Slice3DManager::Slice3DManager(RepresentationPoolSPtr poolXY,
                               RepresentationPoolSPtr poolXZ,
                               RepresentationPoolSPtr poolYZ)
: ActorManager(ViewType::VIEW_3D)
{
  m_pools << poolXY << poolXZ << poolYZ;
}

//----------------------------------------------------------------------------
Slice3DManager::~Slice3DManager()
{
}

//----------------------------------------------------------------------------
TimeRange Slice3DManager::readyRange() const
{
  QMap<TimeStamp, int> count;

  for (auto pool : m_pools)
  {
    for(auto timeStamp: pool->readyRange())
    {
      count[timeStamp] = count.value(timeStamp, 0) + 1;
    }
  }

  TimeRange range;
  for (auto timeStamp : count.keys())
  {
    if (count[timeStamp] == m_pools.size())
    {
      range << timeStamp;
    }
  }

  return range;
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr Slice3DManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  ViewItemAdapterPtr pickedItem = nullptr;

  for (auto pool : m_pools)
  {
    auto pickedItem = pool->pick(point, actor);

    if (pickedItem) break;
  }

  return pickedItem;

}

//----------------------------------------------------------------------------
void Slice3DManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
  for (auto pool : m_pools)
  {
    pool->setCrosshair(crosshair, time);
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t)
{
  for (auto pool : m_pools)
  {
    pool->setResolution(resolution, t);
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors Slice3DManager::actors(TimeStamp time)
{
  RepresentationPipeline::Actors actors;

  for (auto pool : m_pools)
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
void Slice3DManager::invalidatePreviousActors(TimeStamp time)
{
  for (auto pool : m_pools)
  {
    pool->invalidatePreviousActors(time);
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::connectPools()
{
  for (auto pool : m_pools)
  {
    connect(pool.get(), SIGNAL(poolUpdated(TimeStamp)),
            this,       SLOT(waitForDisplay()));

    connect(pool.get(), SIGNAL(actorsReady(TimeStamp)),
            this,       SLOT(checkRenderRequest()));

    connect(pool.get(), SIGNAL(actorsInvalidated()),
            this,       SLOT(invalidateRepresentations()));

    pool->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::disconnectPools()
{
  for (auto pool : m_pools)
  {
    disconnect(pool.get(), SIGNAL(poolUpdated(TimeStamp)),
               this,       SLOT(waitForDisplay()));

    disconnect(pool.get(), SIGNAL(actorsReady(TimeStamp)),
               this,       SLOT(checkRenderRequest()));

    disconnect(pool.get(), SIGNAL(actorsInvalidated()),
               this,       SLOT(invalidateRepresentations()));

    pool->decrementObservers();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::hideActors(TimeStamp t)
{
  for (auto pool : m_pools)
  {
    pool->hideRepresentations(t);
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr Slice3DManager::cloneImplementation()
{
  auto clone = std::make_shared<Slice3DManager>(m_pools[0], m_pools[1], m_pools[2]);

  return clone;
}

//----------------------------------------------------------------------------
void Slice3DManager::checkRenderRequest()
{
  auto lastXY = m_pools[0]->lastUpdateTimeStamp();
  auto lastXZ = m_pools[1]->lastUpdateTimeStamp();
  auto lastYZ = m_pools[2]->lastUpdateTimeStamp();

  auto lastTime = std::min(lastXY, std::min(lastXZ, lastYZ));

  emitRenderRequest(lastTime);
}
