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
{
  m_pools << xy << xz << yz;

  for (auto pool : m_pools)
  {
    connect(pool.get(), SIGNAL(representationsReady()),
            this,       SLOT(onPoolReady()));
  }
}

//----------------------------------------------------------------------------
Slice3DManager::~Slice3DManager()
{
  for (auto pool : m_pools)
  {
    disconnect(pool.get(), SIGNAL(representationsReady()),
               this,       SLOT(onPoolReady()));
  }
}

//----------------------------------------------------------------------------
bool Slice3DManager::isReady() const
{
  for (auto pool : m_pools)
  {
    if (!pool->isReady()) return false;
  }

  return true;
}

//----------------------------------------------------------------------------
void Slice3DManager::onCrosshairChanged(NmVector3 crosshair)
{
//   for (auto pool : m_pools)
//   {
//     pool->setCrosshair(crosshair);
//     pool->update();
//   }
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
RepresentationPipelineSList Slice3DManager::pipelines()
{
  RepresentationPipelineSList pipelines;

  for (auto pool : m_pools)
  {
    pipelines << pool->pipelines();
  }

  return pipelines;
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
void Slice3DManager::onPoolReady()
{
  qDebug() << "Check Pools ";
  for (auto pool : m_pools)
  {
    if (!pool->isReady()) return;
  }

  qDebug() << "Pools ready";

  updateRepresentationActors();
}
