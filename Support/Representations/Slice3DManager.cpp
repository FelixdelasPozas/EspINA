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
Slice3DManager::Slice3DManager()
: ActorManager(ViewType::VIEW_3D)
{
}

//----------------------------------------------------------------------------
Slice3DManager::~Slice3DManager()
{
}

//----------------------------------------------------------------------------
RepresentationManager::PipelineStatus Slice3DManager::pipelineStatus() const
{
  return PipelineStatus::RANGE_DEPENDENT;
}

//----------------------------------------------------------------------------
TimeRange Slice3DManager::readyRange() const
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
void Slice3DManager::setResolution(const NmVector3 &resolution)
{
  for (auto pool : managedPools())
  {
    pool->setResolution(resolution);
  }
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr Slice3DManager::pick(const NmVector3 &point, vtkProp *actor) const
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
bool Slice3DManager::hasSources() const
{
  bool result = true;

  for (auto pool : managedPools())
  {
    result &= pool->hasSources();
  }

  return result;
}

//----------------------------------------------------------------------------
void Slice3DManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
  for (auto pool : managedPools())
  {
    pool->setCrosshair(crosshair, time);
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors Slice3DManager::actors(TimeStamp time)
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
void Slice3DManager::invalidatePreviousActors(TimeStamp time)
{
  for (auto pool : managedPools())
  {
    pool->invalidatePreviousActors(time);
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::connectPools()
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
void Slice3DManager::disconnectPools()
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
RepresentationManagerSPtr Slice3DManager::cloneImplementation()
{
  auto clone = std::make_shared<Slice3DManager>();

  for(auto pool: managedPools())
  {
    clone->addPool(pool);
  }

  return clone;
}

//----------------------------------------------------------------------------
void Slice3DManager::checkRenderRequest()
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
