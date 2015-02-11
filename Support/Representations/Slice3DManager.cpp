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

using namespace ESPINA;

//----------------------------------------------------------------------------
Slice3DManager::Slice3DManager(RepresentationPoolSPtr xy,
                               RepresentationPoolSPtr xz,
                               RepresentationPoolSPtr yz)
: RepresentationManager(ViewType::VIEW_3D)
, m_renderRequestTimeXY{MAX_TIMESTAMP}
, m_renderRequestTimeXZ{MAX_TIMESTAMP}
, m_renderRequestTimeYZ{MAX_TIMESTAMP}
{
  m_pools << xy << xz << yz;
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
    if (count[timeStamp] == 3)
    {
      range << timeStamp;
    }
  }

  return range;
}

//----------------------------------------------------------------------------
void Slice3DManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp time)
{
  for (auto pool : m_pools)
  {
    pool->setCrosshair(crosshair, time);
    pool->update();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::setResolution(const NmVector3 &resolution)
{
  for (auto pool : m_pools)
  {
    pool->setResolution(resolution);
  }
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList Slice3DManager::actors(TimeStamp time)
{
  RepresentationPipeline::ActorList actors;

  for (auto pool : m_pools)
  {
    actors << pool->actors(time);
  }

  return actors;
}

//----------------------------------------------------------------------------
void Slice3DManager::connectPools()
{
  connect(m_pools[0].get(), SIGNAL(actorsReady(TimeStamp)),
          this,             SLOT(onPoolReadyXY(TimeStamp)));
  connect(m_pools[1].get(), SIGNAL(actorsReady(TimeStamp)),
          this,             SLOT(onPoolReadyXZ(TimeStamp)));
  connect(m_pools[2].get(), SIGNAL(actorsReady(TimeStamp)),
          this,             SLOT(onPoolReadyYZ(TimeStamp)));
}

//----------------------------------------------------------------------------
void Slice3DManager::disconnectPools()
{
  disconnect(m_pools[0].get(), SIGNAL(actorsReady(TimeStamp)),
             this,             SLOT(onPoolReadyXY(TimeStamp)));
  disconnect(m_pools[1].get(), SIGNAL(actorsReady(TimeStamp)),
             this,             SLOT(onPoolReadyXZ(TimeStamp)));
  disconnect(m_pools[2].get(), SIGNAL(actorsReady(TimeStamp)),
             this,             SLOT(onPoolReadyYZ(TimeStamp)));
}

//----------------------------------------------------------------------------
void Slice3DManager::notifyPoolUsed()
{
  for (auto pool : m_pools)
  {
    pool->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::notifyPoolNotUsed()
{
  for (auto pool : m_pools)
  {
    pool->decrementObservers();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::updatePipelines()
{
  for (auto pool : m_pools)
  {
    pool->update();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr Slice3DManager::cloneImplementation()
{
  auto clone = std::make_shared<Slice3DManager>(m_pools[0], m_pools[1], m_pools[2]);

  clone->m_name          = m_name;
  clone->m_description   = m_description;
  clone->m_showPipelines = m_showPipelines;

  return clone;
}

//----------------------------------------------------------------------------
void Slice3DManager::checkRenderRequest()
{
  emitRenderRequest(std::min(std::min(m_renderRequestTimeXY, m_renderRequestTimeXZ), m_renderRequestTimeYZ));
}

//----------------------------------------------------------------------------
void Slice3DManager::onPoolReadyXY(TimeStamp time)
{
  m_renderRequestTimeXY = time;
  checkRenderRequest();
}

//----------------------------------------------------------------------------
void Slice3DManager::onPoolReadyXZ(TimeStamp time)
{
  m_renderRequestTimeXZ = time;
  checkRenderRequest();
}

//----------------------------------------------------------------------------
void Slice3DManager::onPoolReadyYZ(TimeStamp time)
{
  m_renderRequestTimeYZ = time;
  checkRenderRequest();
}
