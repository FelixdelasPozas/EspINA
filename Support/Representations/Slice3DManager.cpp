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

// ESPINA
#include "Slice3DManager.h"
#include <GUI/View/RenderView.h>
#include <GUI/Representations/Frame.h>

// VTK
#include <vtkProp.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//----------------------------------------------------------------------------
Slice3DManager::Slice3DManager(RepresentationPoolSPtr poolXY,
                               RepresentationPoolSPtr poolXZ,
                               RepresentationPoolSPtr poolYZ,
                               ManagerFlags           flags)
: PoolManager(ViewType::VIEW_3D, flags)
{
  m_pools << poolXY << poolXZ << poolYZ;
}

//----------------------------------------------------------------------------
ViewItemAdapterList Slice3DManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  ViewItemAdapterList pickedItems;

  for (auto pool : m_pools)
  {
    auto items = pool->pick(point, actor);

    pickedItems << items;
  }

  return pickedItems;
}

//----------------------------------------------------------------------------
bool Slice3DManager::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return RepresentationManager::acceptCrosshairChange(crosshair);
}

//----------------------------------------------------------------------------
bool Slice3DManager::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return RepresentationManager::acceptSceneResolutionChange(resolution);
}

//----------------------------------------------------------------------------
bool Slice3DManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return RepresentationManager::acceptSceneBoundsChange(bounds);
}

//----------------------------------------------------------------------------
bool Slice3DManager::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  bool result = false;

  for(auto pool: m_pools)
  {
    result |= invalidatesRepresentations(frame, pool->type());
  }

  return result;
}

//----------------------------------------------------------------------------
bool Slice3DManager::hasRepresentations() const
{
  bool result = false;

  for (auto pool : m_pools)
  {
    result |= pool->hasSources();
  }

  return result;
}

//----------------------------------------------------------------------------
void Slice3DManager::updateFrameRepresentations(const FrameCSPtr frame)
{
  for (auto pool : m_pools)
  {
    pool->updatePipelines(frame);
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::onShow(const FrameCSPtr frame)
{
  connectPools();
}

//----------------------------------------------------------------------------
void Slice3DManager::onHide(const FrameCSPtr frame)
{
  disconnectPools();
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors Slice3DManager::actors(TimeStamp time)
{
  auto actors = std::make_shared<RepresentationPipeline::ActorsData>();
  RepresentationPipeline::ActorsLocker returnActors(actors);

  for (auto pool : m_pools)
  {
    auto frameActors = pool->actors(time);

    if(frameActors)
    {
      RepresentationPipeline::ActorsLocker poolActors(frameActors);

      for(auto it = poolActors.get().begin(); it != poolActors.get().end(); ++it)
      {
        returnActors.get()[it.key()] << it.value();
      }
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
void Slice3DManager::invalidatePreviousActors(TimeStamp time)
{
  for(auto pool: m_pools)
  {
    pool->invalidatePreviousActors(time);
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::connectPools()
{
  for (auto pool : m_pools)
  {
    connect(pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
            this,       SLOT(onActorsInvalidated(GUI::Representations::FrameCSPtr)), Qt::DirectConnection);

    connect(pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
            this,       SLOT(checkRenderRequest(GUI::Representations::FrameCSPtr)));

    pool->incrementObservers();
  }
}

//----------------------------------------------------------------------------
void Slice3DManager::disconnectPools()
{
  for (auto pool : m_pools)
  {
    disconnect(pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
               this,       SLOT(onActorsInvalidated(GUI::Representations::FrameCSPtr)));

    disconnect(pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
               this,       SLOT(checkRenderRequest(GUI::Representations::FrameCSPtr)));

    pool->decrementObservers();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr Slice3DManager::cloneImplementation()
{
  return std::make_shared<Slice3DManager>(m_pools[0], m_pools[1], m_pools[2], flags());
}

//----------------------------------------------------------------------------
RepresentationPoolSList Slice3DManager::pools() const
{
  return m_pools;
}

//----------------------------------------------------------------------------
void Slice3DManager::checkRenderRequest(const GUI::Representations::FrameCSPtr frame)
{
  if (!isIdle())
  {
    auto lastXY = m_pools[0]->lastUpdateTimeStamp();
    auto lastXZ = m_pools[1]->lastUpdateTimeStamp();
    auto lastYZ = m_pools[2]->lastUpdateTimeStamp();

    auto lastTime = std::min(lastXY, std::min(lastXZ, lastYZ));

    if (isValid(frame) && lastTime == frame->time)
    {
      emitRenderRequest(frame);
    }
  }
}
