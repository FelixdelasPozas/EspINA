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
#include <GUI/Representations/Managers/PassiveActorManager.h>
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
TimeRange PassiveActorManager::readyRangeImplementation() const
{
  return m_pool->readyRange();
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr PassiveActorManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  return m_pool->pick(point, actor);
}

//----------------------------------------------------------------------------
bool PassiveActorManager::hasRepresentations() const
{
  return m_pool->hasSources();
}

//----------------------------------------------------------------------------
void PassiveActorManager::updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t)
{
  m_pool->updatePipelines(crosshair, resolution, t);
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
void PassiveActorManager::onShow(TimeStamp t)
{
  connectPools();
}

//----------------------------------------------------------------------------
void PassiveActorManager::onHide(TimeStamp t)
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
  connect(m_pool.get(), SIGNAL(actorsInvalidated()),
          this,         SLOT(waitForDisplay()));

  connect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
          this,         SLOT(emitRenderRequest(TimeStamp)));

  connect(m_pool.get(), SIGNAL(actorsInvalidated()),
          this,         SLOT(invalidateRepresentations()));

  m_pool->incrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::disconnectPools()
{
  disconnect(m_pool.get(), SIGNAL(actorsInvalidated()),
             this,         SLOT(waitForDisplay()));

  disconnect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
             this,         SLOT(emitRenderRequest(TimeStamp)));

  disconnect(m_pool.get(), SIGNAL(actorsInvalidated()),
             this,         SLOT(invalidateRepresentations()));

  m_pool->decrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::displayRepresentations(const TimeStamp t)
{
  auto currentActors = m_viewActors;
  auto futureActors  = m_pool->actors(t);

  qDebug() << "passive display" << t << "current" << currentActors.size() << "future" << futureActors.size();

  if(currentActors.empty())
  {
    m_viewActors = futureActors;
    for(auto item: m_viewActors.keys())
    {
      for(auto actor: m_viewActors[item])
      {
        m_view->addActor(actor);
      }
    }
  }
  else
  {
    for (auto it = currentActors.begin(); it != currentActors.end(); ++it)
    {
      auto item = it.key();

      auto oldActors = toSet(it.value());
      auto newActors = toSet(futureActors[item]);

      if(oldActors != newActors)
      {
        for (auto oldActor : oldActors.subtract(newActors))
        {
          m_view->removeActor(oldActor);
        }

        for (auto newActor : newActors.subtract(oldActors))
        {
          m_view->addActor(newActor);
        }

        m_viewActors[item] = futureActors[item];
      }
    }
  }

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
QSet<vtkProp *> PassiveActorManager::toSet(const RepresentationPipeline::ActorList &list) const
{
  QSet<vtkProp *> set;

  for (auto actor : list)
  {
    set.insert(actor.Get());
  }

  return set;
}
