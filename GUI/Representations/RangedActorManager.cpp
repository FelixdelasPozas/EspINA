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
#include <GUI/Representations/RangedActorManager.h>
#include <GUI/View/RenderView.h>

#include <vtkProp.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RangedActorManager::RangedActorManager(ViewTypeFlags supportedViews)
: RepresentationManager{supportedViews}
{
}

//-----------------------------------------------------------------------------
void RangedActorManager::display(TimeStamp time)
{
  if (m_view)
  {
    removeCurrentActors();

    if (m_showRepresentations)
    {
      displayActors(time);
    }

    invalidatePreviousActors(time);

    m_requiresRender = m_showRepresentations && hasSources();
  }
}

//-----------------------------------------------------------------------------
void RangedActorManager::onShow()
{
  enableRepresentations();
}

//-----------------------------------------------------------------------------
void RangedActorManager::onHide()
{
  disableRepresentations();
}

//-----------------------------------------------------------------------------
void RangedActorManager::enableRepresentations()
{
  m_requiresRender = hasSources();

  connectPools();
}

//-----------------------------------------------------------------------------
void RangedActorManager::disableRepresentations()
{
  disconnectPools();
}

//-----------------------------------------------------------------------------
void RangedActorManager::removeCurrentActors()
{
  for (auto itemActors : m_viewActors)
  {
    for (auto actor : itemActors)
    {
      m_view->removeActor(actor);
    }
  }

  m_viewActors.clear();
}

//-----------------------------------------------------------------------------
void RangedActorManager::displayActors(const TimeStamp time)
{
  if(readyRange().empty()) return;

  auto currentActors = actors(time);

  for(auto it = currentActors.begin(); it != currentActors.end(); ++it)
  {
    for (auto actor : it.value())
    {
      m_view->addActor(actor);
      m_viewActors[it.key()] << actor;
    }
  }
}

//-----------------------------------------------------------------------------
bool RangedActorManager::hasActorsInDisplay() const
{
  return !m_viewActors.empty();
}
