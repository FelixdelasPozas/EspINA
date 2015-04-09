/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include <GUI/Representations/ActorManager.h>
#include <GUI/View/RenderView.h>

#include <vtkProp.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ActorManager::ActorManager(ViewTypeFlags supportedViews)
: RepresentationManager{supportedViews}
{
}

//-----------------------------------------------------------------------------
void ActorManager::displayImplementation(TimeStamp t)
{
  if (representationsShown())
  {
    displayActors(t);
  }
  else
  {
    removeCurrentActors();
  }

  invalidatePreviousActors(t);
}

//-----------------------------------------------------------------------------
void ActorManager::onShow(TimeStamp t)
{
  connectPools();
}

//-----------------------------------------------------------------------------
void ActorManager::onHide(TimeStamp t)
{
  hideActors(t);
  disconnectPools();
}

//-----------------------------------------------------------------------------
void ActorManager::displayActors(const TimeStamp time)
{
  removeCurrentActors();

  if (representationsShown())
  {
    auto currentActors = actors(time);

    for(auto it = currentActors.begin(); it != currentActors.end(); ++it)
    {
      for (auto actor : it.value())
      {
        setFlag(HAS_ACTORS, true);
        m_view->addActor(actor);
        m_viewActors[it.key()] << actor;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void ActorManager::removeCurrentActors()
{
  setFlag(HAS_ACTORS, false);

  for (auto itemActors : m_viewActors)
  {
    for (auto actor : itemActors)
    {
      m_view->removeActor(actor);
    }
  }

  m_viewActors.clear();
}