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
  return false;
}

//----------------------------------------------------------------------------
bool PassiveActorManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
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
RepresentationPipeline::Actors PassiveActorManager::actors(TimeStamp t)
{
  return m_pool->actors(t);
}

//----------------------------------------------------------------------------
void PassiveActorManager::invalidatePreviousActors(TimeStamp t)
{
  m_pool->invalidatePreviousActors(t);
}

//----------------------------------------------------------------------------
void PassiveActorManager::connectPools()
{
  connect(m_pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
          this,         SLOT(waitForDisplay(GUI::Representations::FrameCSPtr)));

  connect(m_pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
          this,         SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  qDebug() << debugName() << "Activating representation pools";
  m_pool->incrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::disconnectPools()
{
  disconnect(m_pool.get(), SIGNAL(actorsInvalidated(GUI::Representations::FrameCSPtr)),
             this,         SLOT(waitForDisplay(GUI::Representations::FrameCSPtr)));

  disconnect(m_pool.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr)),
             this,         SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  qDebug() << debugName() << "Dectivating representation pools";
  m_pool->decrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::displayRepresentations(const FrameCSPtr frame)
{
  auto currentActors = m_viewActors;
  auto futureActors  = m_pool->actors(frame->time);

  auto currentSegs = currentActors.keys().toSet();
  auto futureSegs  = futureActors.keys().toSet();

  auto temporalSet = currentSegs;
  auto toRemove    = temporalSet.subtract(futureSegs);

  temporalSet    = currentSegs;
  auto toCompare = temporalSet.intersect(futureSegs);

  temporalSet    = futureSegs;
  auto toAdd     = temporalSet.subtract(currentSegs);

  // remove
  for(auto item: toRemove)
  {
    for(auto actor: currentActors[item])
    {
      m_view->removeActor(actor);
    }

    m_viewActors.remove(item);
  }

  // compare
  for(auto item: toCompare)
  {
    auto oldActors = toSet(m_viewActors[item]);
    auto newActors = toSet(futureActors[item]);

    if(oldActors != newActors)
    {
      auto temporalSet = oldActors;
      for (auto oldActor : temporalSet.subtract(newActors))
      {
        m_view->removeActor(oldActor);
      }

      temporalSet = newActors;
      for (auto newActor : temporalSet.subtract(oldActors))
      {
        m_view->addActor(newActor);
      }
    }

    m_viewActors[item] = futureActors[item];
  }

  // add
  for(auto item: toAdd)
  {
    for(auto actor: futureActors[item])
    {
      m_view->addActor(actor);
    }

    m_viewActors.insert(item, futureActors[item]);
  }

  // set flag
  for(auto actors : m_viewActors)
  {
    if(!actors.isEmpty())
    {
      setFlag(HAS_ACTORS, true);
      break;
    }
  }
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

//----------------------------------------------------------------------------
QSet<vtkProp *> PassiveActorManager::toSet(const RepresentationPipeline::ActorList &list) const
{
  QSet<vtkProp *> set;

  for (auto actor : list)
  {
    set.insert(actor.Get());
  }

  return set;
}
