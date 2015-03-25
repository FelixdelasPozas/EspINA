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
#include <GUI/Representations/ReadyActorManager.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkProp.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  ReadyActorManager::ReadyActorManager(RepresentationPoolSPtr pool, ViewTypeFlags supportedViews)
  : RepresentationManager{supportedViews}
  , m_pool               {pool}
  , m_init               {false}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationManager::PipelineStatus ReadyActorManager::pipelineStatus() const
  {
    return PipelineStatus::READY;
  }

  //----------------------------------------------------------------------------
  TimeRange ReadyActorManager::readyRange() const
  {
    Q_ASSERT(false);
    return TimeRange();
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::setResolution(const NmVector3 &resolution)
  {
  }

  //----------------------------------------------------------------------------
  ViewItemAdapterPtr ReadyActorManager::pick(const NmVector3 &point, vtkProp *actor) const
  {
    return m_pool->pick(point, actor);
  }

  //----------------------------------------------------------------------------
  bool ReadyActorManager::hasSources() const
  {
    return m_pool->hasSources();
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
  {
    if(!m_init)
    {
      m_pool->setCrosshair(crosshair, time);
      m_init = true;
    }
  }

  //-----------------------------------------------------------------------------
  void ReadyActorManager::display(TimeStamp time)
  {
    if (m_view)
    {
      if(m_showRepresentations)
      {
        displayActors();
      }
      else
      {
        removeActors();
      }

      m_requiresRender = m_showRepresentations && hasSources();
    }
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::connectPools()
  {
    connect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
            this,         SLOT(emitRenderRequest(TimeStamp)));

    connect(m_pool.get(), SIGNAL(actorsInvalidated()),
            this,         SLOT(invalidateRepresentations()));

    m_pool->incrementObservers();
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::disconnectPools()
  {
    disconnect(m_pool.get(), SIGNAL(actorsReady(TimeStamp)),
               this,         SLOT(emitRenderRequest(TimeStamp)));

    disconnect(m_pool.get(), SIGNAL(actorsInvalidated()),
              this,         SLOT(invalidateRepresentations()));

    m_pool->decrementObservers();
  }

  //----------------------------------------------------------------------------
  RepresentationManagerSPtr ReadyActorManager::cloneImplementation()
  {
    auto clone = std::make_shared<ReadyActorManager>(m_pool, supportedViews());

    return clone;
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::displayActors()
  {
    if(m_pool->readyRange().empty()) return;

    auto currentActors = m_pool->actors(lastValidTimeStamp());

    if(!currentActors.empty())
    {
      for(auto it = currentActors.begin(); it != currentActors.end(); ++it)
      {
        auto item = it.key();

        if(m_actors[item] != it.value())
        {
          for(auto actor: m_actors[item])
          {
            m_view->removeActor(actor);
          }
          m_actors[item].clear();

          for(auto actor: it.value())
          {
            m_view->addActor(actor);
            m_actors[item] << actor;
          }
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::removeActors()
  {
    for(auto it = m_actors.begin(); it != m_actors.end(); ++it)
    {
      for(auto actor: it.value())
      {
        m_view->removeActor(actor);
      }
    }

    m_actors.clear();
  }

  //----------------------------------------------------------------------------
  TimeStamp ReadyActorManager::lastValidTimeStamp() const
  {
    return m_pool->readyRange().last();
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::onShow()
  {
    m_requiresRender = hasSources();

    connectPools();

    displayActors();
  }

  //----------------------------------------------------------------------------
  void ReadyActorManager::onHide()
  {
    removeActors();

    disconnectPools();
  }

} // namespace ESPINA
