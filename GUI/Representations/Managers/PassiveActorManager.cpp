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
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Representations/Managers/PassiveActorManager.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Frame.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkProp.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//----------------------------------------------------------------------------
PassiveActorManager::PassiveActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews, ManagerFlags flags)
: PoolManager{supportedViews, flags}
, m_pool     {pool}
{
}

//----------------------------------------------------------------------------
ViewItemAdapterList PassiveActorManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  return m_pool->pick(point, actor);
}

//----------------------------------------------------------------------------
bool PassiveActorManager::hasRepresentations() const
{
  return m_pool->hasSources();
}

//----------------------------------------------------------------------------
void PassiveActorManager::updateFrameRepresentations(const FrameCSPtr frame)
{
  m_pool->updatePipelines(frame);
}

//----------------------------------------------------------------------------
bool PassiveActorManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return false;
}

//----------------------------------------------------------------------------
bool PassiveActorManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return !isValid(lastFrame()) || (lastFrame()->resolution != resolution);
}

//----------------------------------------------------------------------------
bool PassiveActorManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return !isValid(lastFrame()) || (lastFrame()->bounds != bounds);
}

//----------------------------------------------------------------------------
bool PassiveActorManager::acceptInvalidationFrame(const FrameCSPtr frame) const
{
  auto type = m_pool->type();

  return invalidatesRepresentations(frame, type);
}

//----------------------------------------------------------------------------
void PassiveActorManager::onShow(const FrameCSPtr frame)
{
  connectPools();
}

//----------------------------------------------------------------------------
void PassiveActorManager::onHide(const FrameCSPtr frame)
{
  disconnectPools();
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors PassiveActorManager::actors(TimeStamp time)
{
  return m_pool->actors(time);
}

//----------------------------------------------------------------------------
void PassiveActorManager::invalidatePreviousActors(TimeStamp time)
{
  m_pool->invalidatePreviousActors(time);
}

//----------------------------------------------------------------------------
void PassiveActorManager::connectPools()
{
  connect(m_pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
          this,         SLOT(onActorsInvalidated(GUI::Representations::FrameCSPtr)));

  connect(m_pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
          this,         SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  m_pool->incrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::disconnectPools()
{
  disconnect(m_pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
             this,         SLOT(onActorsInvalidated(GUI::Representations::FrameCSPtr)));

  disconnect(m_pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
             this,         SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  m_pool->decrementObservers();
}

//----------------------------------------------------------------------------
QSet<vtkProp *> toRawSet(QList<vtkSmartPointer<vtkProp>> list)
{
  QSet<vtkProp *> result;

  for(auto item: list)
  {
    result.insert(item.GetPointer());
  }

  return result;
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr PassiveActorManager::cloneImplementation()
{
  return std::make_shared<PassiveActorManager>(m_pool, supportedViews(), flags());
}

//----------------------------------------------------------------------------
RepresentationPoolSList PassiveActorManager::pools() const
{
  RepresentationPoolSList result;
  result << m_pool;

  return result;
}
