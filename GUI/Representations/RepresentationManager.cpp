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
using namespace ESPINA::GUI::Representations;

//-----------------------------------------------------------------------------
RepresentationManager::RepresentationManager(ViewTypeFlags supportedViews, ManagerFlags flags)
: m_view{nullptr}
, m_isActive{false}
, m_status{Status::IDLE}
, m_flags {flags}
, m_supportedViews{supportedViews}
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
bool RepresentationManager::hasActors() const
{
  return m_flags.testFlag(HAS_ACTORS);
}

//-----------------------------------------------------------------------------
bool RepresentationManager::needsActors() const
{
  return m_flags.testFlag(NEEDS_ACTORS);
}

//-----------------------------------------------------------------------------
bool RepresentationManager::exports3D() const
{
  return m_flags.testFlag(EXPORTS_3D);
}

//-----------------------------------------------------------------------------
RepresentationManager::ManagerFlags RepresentationManager::flags() const
{
  return m_flags;
}

//-----------------------------------------------------------------------------
void RepresentationManager::setView(RenderView *view)
{
  m_view = view;

  auto t = view->timeStamp();

  m_crosshair  = m_view->crosshair();
  m_resolution = m_view->sceneResolution();
  m_bounds     = m_view->sceneBounds();

  if (m_isActive)
  {
    updateRepresentations(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show(TimeStamp t)
{
  m_isActive = true;

  if (m_view)
  {
    updateRepresentations(t);
  }

  for (auto child : m_childs)
  {
    child->show(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide(TimeStamp t)
{
  m_isActive = false;

  if (m_view)
  {
    onHide(t);

    //qDebug() << debugName() << "Requested hide" << t;

    if(hasRepresentations())
    {
      waitForDisplay();

      emitRenderRequest(t);
    }
  }

  for (auto child : m_childs)
  {
    child->hide(t);
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isActive() const
{
  return m_isActive && m_view;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::isIdle() const
{
  return m_status == Status::IDLE;
}

//-----------------------------------------------------------------------------
TimeRange RepresentationManager::readyRange() const
{
  TimeRange range;

  if (isActive())
  {
    range = readyRangeImplementation();
  }
  else
  {
    range << m_lastRenderRequestTime;
  }

  return range;
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(TimeStamp t)
{
  Q_ASSERT(m_view);

  if (isActive())
  {
    //qDebug() << debugName() << "Display" << t << "actors";
    displayRepresentations(t);
  }
  else
  {
    //qDebug() << debugName() << "Hide at" << t;
    hideRepresentations(t);
  }

  if (!hasNewerFrames(t))
  {
    //qDebug() << debugName() << "Displayed las frame" << t;
    idle();
  }
//   else
//   {
//     qDebug() << debugName() << "Pending Frames at" << t;
//   }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager::clone()
{
  auto child = cloneImplementation();

  child->m_icon        = m_icon;
  child->m_name        = m_name;
  child->m_description = m_description;
  child->m_isActive    = m_isActive;

  m_childs << child;

  return child;
}

//-----------------------------------------------------------------------------
QString RepresentationManager::debugName() const
{
  return m_view->viewName() + "::" + name() + ":";
}

//-----------------------------------------------------------------------------
void RepresentationManager::onCrosshairChanged(NmVector3 crosshair, TimeStamp t)
{
  if (acceptCrosshairChange(crosshair))
  {
    m_crosshair = crosshair;

    if (isActive())
    {
      if(hasRepresentations())
      {
        waitForDisplay();
      }

      changeCrosshair(crosshair, t);
    }
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t)
{
  if (acceptSceneResolutionChange(resolution))
  {
    m_resolution = resolution;

    if (isActive())
    {
      if(hasRepresentations())
      {
        waitForDisplay();
      }

      changeSceneResolution(m_resolution, t);
    }
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::onSceneBoundsChanged(const Bounds &bounds, TimeStamp t)
{
  if (acceptSceneBoundsChange(bounds))
  {
    m_bounds = bounds;

    if (isActive())
    {
      if(hasRepresentations())
      {
        waitForDisplay();
      }

      changeSceneBounds(m_bounds, t);
    }
  }
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
NmVector3 RepresentationManager::currentCrosshair() const
{
  return m_crosshair;
}

//-----------------------------------------------------------------------------
NmVector3 RepresentationManager::currentSceneResolution() const
{
  return m_resolution;
}

//-----------------------------------------------------------------------------
Bounds RepresentationManager::currentSceneBounds() const
{
  return m_bounds;
}

//-----------------------------------------------------------------------------
void RepresentationManager::emitRenderRequest(TimeStamp t)
{
  //qDebug() << debugName() << "Requested to emit renderRequested at" << t;
  if(t > m_lastRenderRequestTime)
  {
    m_lastRenderRequestTime = t;

    //qDebug() << debugName() << "Emit renderRequested at" << t;

    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::invalidateRepresentations()
{
  // qDebug() << debugName() << "last render timestamp invalid";
  m_lastRenderRequestTime = Timer::INVALID_TIME_STAMP;
}

//-----------------------------------------------------------------------------
void RepresentationManager::waitForDisplay()
{
  m_status = Status::PENDING_DISPLAY;
}

//-----------------------------------------------------------------------------
void RepresentationManager::idle()
{
  //qDebug() << debugName() << "Is idle";
  m_status = Status::IDLE;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return m_crosshair != crosshair;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return m_resolution != resolution;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return m_bounds != bounds;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::hasNewerFrames(TimeStamp t) const
{
  return t < m_lastRenderRequestTime;
}

//-----------------------------------------------------------------------------
void RepresentationManager::updateRepresentations(TimeStamp t)
{
  onShow(t);

  if (hasRepresentations())
  {
    waitForDisplay();
  }

  updateRepresentations(m_crosshair, m_resolution, m_bounds, t);
}
