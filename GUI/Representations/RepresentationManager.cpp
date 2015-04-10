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
, m_representationsShown{false}
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
RepresentationManager::Flags RepresentationManager::flags() const
{
  return m_flags;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setView(RenderView *view)
{
  m_view = view;

  auto t = view->timeStamp();

  onSceneResolutionChanged(view->sceneResolution(), t);
  onSceneBoundsChanged(view->sceneBounds(), t);

  if (m_representationsShown)
  {
    showRepresentations(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show(TimeStamp t)
{
  for (auto child : m_childs)
  {
    child->show(t);
  }

  m_representationsShown  = true;

  if (m_view)
  {
    showRepresentations(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide(TimeStamp t)
{
  for (auto child : m_childs)
  {
    child->hide(t);
  }

  m_representationsShown = false;

  if (m_view)
  {
    qDebug() << debugName() << "Requested hide" << t;
    waitForDisplay();

    onHide(t);
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isActive()
{
  return m_representationsShown && m_view;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isIdle() const
{
  return m_status == Status::IDLE;
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(TimeStamp t)
{
  Q_ASSERT(m_view);

  qDebug() << debugName() << "Display" << t;
  displayImplementation(t);

  if (!hasNewerFrames(t))
  {
    qDebug() << debugName() << "Displayed las frame" << t;
    idle();
  }
  else
  {
    qDebug() << debugName() << "PENDING Frames at" << t;
  }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager::clone()
{
  auto child = cloneImplementation();

  child->m_name                = m_name;
  child->m_description         = m_description;
  child->m_representationsShown = m_representationsShown;

  m_childs << child;

  return child;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::debugName() const
{
  return m_view->viewName() + "::" + name() + ":";
}

//-----------------------------------------------------------------------------
void RepresentationManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp time)
{
  m_crosshair       = crosshair;
  m_lastRequestTime = time; // Remove?

  if (representationsShown())
  {
    changeCrosshair(m_crosshair, time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t)
{
  changeSceneResolution(resolution, t);
}


//-----------------------------------------------------------------------------
void RepresentationManager::onSceneBoundsChanged(const Bounds &bounds, TimeStamp t)
{
  changeSceneBounds(bounds, t);
}

//-----------------------------------------------------------------------------
bool RepresentationManager::representationsShown() const
{
  return m_representationsShown;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setFlag(const FlagValue flag, const bool value)
{
  if (value)
  {
    m_flags |= flag;
  }
  else
  {
    m_flags &= !flag;
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::emitRenderRequest(TimeStamp t)
{

  qDebug() << debugName() << "Requested to emit renderRequested at" << t;
  if(t > m_lastRenderRequestTime)
  {
    Q_ASSERT(t == readyRange().last());

    m_lastRenderRequestTime = t;

    qDebug() << debugName() << "Emit renderRequested at" << t;

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
  qDebug() << debugName() << "Wait for display";
  m_status = Status::PENDING_DISPLAY;
}

//-----------------------------------------------------------------------------
void RepresentationManager::idle()
{
  qDebug() << debugName() << "Is idle";
  m_status = Status::IDLE;
}


//-----------------------------------------------------------------------------
bool RepresentationManager::hasNewerFrames(TimeStamp t) const
{
  return t < m_lastRenderRequestTime;
}

//-----------------------------------------------------------------------------
void RepresentationManager::showRepresentations(TimeStamp t)
{
  onShow(t);

  m_lastRequestTime = t;

  changeCrosshair(m_crosshair, m_lastRequestTime);

  waitForDisplay();
}
