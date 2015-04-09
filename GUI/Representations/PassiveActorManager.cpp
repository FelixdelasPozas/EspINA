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
#include <GUI/Representations/PassiveActorManager.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkProp.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
PassiveActorManager::PassiveActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews)
: ActorManager{supportedViews}
, m_pool      {pool}
{
}

//----------------------------------------------------------------------------
TimeRange PassiveActorManager::readyRange() const
{
  TimeRange range;

  range << m_pool->lastUpdateTimeStamp();

  return range;
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr PassiveActorManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  return m_pool->pick(point, actor);
}

//----------------------------------------------------------------------------
void PassiveActorManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
{
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
  connect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
          this,         SLOT(emitRenderRequest(TimeStamp)));

  connect(m_pool.get(), SIGNAL(actorsInvalidated()),
          this,         SLOT(invalidateRepresentations()));

  m_pool->incrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::disconnectPools()
{
  disconnect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
             this,         SLOT(emitRenderRequest(TimeStamp)));

  disconnect(m_pool.get(), SIGNAL(actorsInvalidated()),
             this,         SLOT(invalidateRepresentations()));

  m_pool->decrementObservers();
}

//----------------------------------------------------------------------------
void PassiveActorManager::hideActors(TimeStamp t)
{
  m_pool->hideRepresentations(t);
}

//----------------------------------------------------------------------------
void PassiveActorManager::displayActors(const TimeStamp t)
{
  auto currentActors = m_pool->actors(t);

  for (auto it = currentActors.begin(); it != currentActors.end(); ++it)
  {
    auto item = it.key();

    auto previousActors = toSet(m_viewActors[item]);
    auto itemActors     = toSet(it.value());

    for (auto oldActor : previousActors.subtract(itemActors))
    {
      m_view->removeActor(oldActor);
    }

    for (auto newActor : itemActors.subtract(previousActors))
    {
      m_view->addActor(newActor);
    }

    m_viewActors[item] = it.value();
  }
}

//----------------------------------------------------------------------------
RepresentationManagerSPtr PassiveActorManager::cloneImplementation()
{
  return std::make_shared<PassiveActorManager>(m_pool, supportedViews());
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