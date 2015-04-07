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
#include "RepresentationManager.h"
#include <GUI/View/RenderView.h>

#include <vtkProp.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationManager::RepresentationManager(ViewTypeFlags supportedViews)
: m_view{nullptr}
, m_showRepresentations{false}
, m_status{Status::IDLE}
, m_supportedViews{supportedViews}
, m_lastRequestTime{Timer::INVALID_TIME_STAMP}
, m_lastRenderRequestTime{Timer::INVALID_TIME_STAMP}
{
}

//-----------------------------------------------------------------------------
void RepresentationManager::setName(const QString &name)
{
  m_name = name;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::name() const
{
  return m_name;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setDescription(const QString &description)
{
  m_description = description;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::description() const
{
  return m_description;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setIcon(const QIcon &icon)
{
  m_icon = icon;
}

//-----------------------------------------------------------------------------
QIcon RepresentationManager::icon() const
{
  return m_icon;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setView(RenderView *view)
{
  m_view = view;

  auto t = view->timeStamp();

  onSceneResolutionChanged(view->sceneResolution(), t);
  onSceneBoundsChanged(view->sceneBounds(), t);

  if (m_showRepresentations)
  {
    onShow();

    m_lastRequestTime = t;

    setCrosshair(m_crosshair, m_lastRequestTime);

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show()
{
  for (auto child : m_childs)
  {
    child->show();
  }

  m_showRepresentations  = true;

  if (m_view)
  {
    onShow();

    m_lastRequestTime = m_view->timeStamp();

    setCrosshair(m_crosshair, m_lastRequestTime);

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide()
{
  for (auto child : m_childs)
  {
    child->hide();
  }

  m_showRepresentations = false;

  if (m_view)
  {
    onHide();

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isActive()
{
  return m_showRepresentations && m_view;
}

//-----------------------------------------------------------------------------
RepresentationManager::Status RepresentationManager::status() const
{
  return m_status;
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(TimeStamp t)
{
  if (m_view)
  {
    displayImplementation(t);
  }

  if (!hasNewerFrames(t))
  {
    m_status = Status::IDLE;
  }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager::clone()
{
  auto child = cloneImplementation();

  child->m_name                = m_name;
  child->m_description         = m_description;
  child->m_showRepresentations = m_showRepresentations;

  m_childs << child;

  return child;
}

//-----------------------------------------------------------------------------
void RepresentationManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp time)
{
  m_crosshair       = crosshair;
  m_lastRequestTime = time;

  if (m_showRepresentations)
  {
    setCrosshair(m_crosshair, time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::setRepresentationsVisibility(bool value)
{
  if (m_showRepresentations != value)
  {
    if (value)
    {
      show();
    }
    else
    {
      hide();
    }
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::representationsShown() const
{
  return m_showRepresentations;
}

//-----------------------------------------------------------------------------
void RepresentationManager::emitRenderRequest(TimeStamp time)
{
  if(time > m_lastRenderRequestTime)
  {
    m_lastRenderRequestTime = time;

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::invalidateRepresentations()
{
  m_lastRenderRequestTime = 0;
}

//-----------------------------------------------------------------------------
void RepresentationManager::waitForDisplay()
{
  m_status = Status::PENDING_DISPLAY;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::hasNewerFrames(TimeStamp t) const
{
  return t < m_lastRenderRequestTime;
}