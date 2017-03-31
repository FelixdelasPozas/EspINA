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
#include <GUI/Representations/Managers/PoolManager.h>
#include <GUI/Representations/Frame.h>
#include <GUI/View/RenderView.h>

// Qt
#include <vtkProp.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//-----------------------------------------------------------------------------
PoolManager::PoolManager(ViewTypeFlags supportedViews, ManagerFlags flags)
: RepresentationManager{supportedViews, flags}
{
}

//-----------------------------------------------------------------------------
void PoolManager::displayRepresentations(const FrameCSPtr frame)
{
  hideRepresentations(frame);

  auto frameActors = actors(frame->time);

  if(frameActors.get() != nullptr)
  {
    RepresentationPipeline::ActorsLocker actors(frameActors);

    for (auto itemActors: actors.get())
    {
      for (auto actor: itemActors)
      {
        setFlag(HAS_ACTORS, true);
        m_view->addActor(actor);
        m_viewActors << actor;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void PoolManager::hideRepresentations(const FrameCSPtr frame)
{
  for (auto actor : m_viewActors)
  {
    m_view->removeActor(actor);
  }

  setFlag(HAS_ACTORS, false);

  m_viewActors.clear();
}

//-----------------------------------------------------------------------------
void PoolManager::onActorsInvalidated(const GUI::Representations::FrameCSPtr frame)
{
  invalidateFrames(frame);

  waitForDisplay(frame);
}
