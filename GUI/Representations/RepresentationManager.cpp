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
#include "Frame.h"
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
, m_lastFrameChanged{Timer::INVALID_TIME_STAMP}
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
void RepresentationManager::setView(RenderView *view, const FrameCSPtr frame)
{
  m_view = view;

  if (isActive())
  {
    if (hasRepresentations())
    {
      updateRepresentations(frame);
    }

    onShow(frame);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show(const GUI::Representations::FrameCSPtr frame)
{
  m_isActive = true;

  if (isActive() && hasRepresentations())
  {
    updateRepresentations(frame);
  }

  for (auto child : m_childs)
  {
    child->show(frame);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::hide(const GUI::Representations::FrameCSPtr frame)
{
  m_isActive = false;

  if (m_view)
  {
    onHide(frame);

    if(hasRepresentations())
    {
      waitForDisplay(frame);

      emitRenderRequest(frame);
    }
  }

  for (auto child : m_childs)
  {
    child->hide(frame);
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
  return m_frames.timeRange();
}

//-----------------------------------------------------------------------------
FrameCSPtr RepresentationManager::frame(TimeStamp t) const
{
  auto result = Frame::InvalidFrame();

  if (!m_frames.isEmpty() && t <= m_frames.lastTime())
  {
    result = m_frames.value(t);
  }

  return result;
}

//-----------------------------------------------------------------------------
FrameCSPtr RepresentationManager::lastFrame() const
{
  return frame(m_frames.lastTime());
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(TimeStamp time)
{
  Q_ASSERT(m_view);

  qDebug() << "\t" << debugName() << "rendering" << time;

  if(m_frames.value(time, Frame::InvalidFrame()) == Frame::InvalidFrame())
  {
    qDebug() << debugName() << "FAIL: requesting invalid frame" << time;
    return;
  }

  if (isActive())
  {
    qDebug() << "\t" << debugName() << "Display actors at" << time << "corresponding frame" << m_frames.value(time)->time;
    displayRepresentations(m_frames.value(time));

    m_frames.invalidatePreviousValues(time);
  }
  else
  {
    qDebug() << "\t" << debugName() << "Hide frame at" << time;
    auto last = lastFrame();
    hideRepresentations(last);
    m_frames.invalidatePreviousValues(last->time);
  }

  if (!waitingNewerFrames(time))
  {
    qDebug() << "\t" << debugName() << "Displayed last frame" << time << "going idle";
    idle();
  }
  else
  {
    qDebug() << "\t" << debugName() << "Waiting from frame" << m_lastFrameChanged;
  }
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
void RepresentationManager::onFrameChanged(const FrameCSPtr frame)
{
  qDebug() << debugName() << "received frame" << frame->time;
  if (!isValid(frame)) return;

  if (isActive())
  {
    if (needsRepresentationUpdate(frame))
    {
      if(hasRepresentations())
      {
        waitForDisplay(frame);
      }

      updateFrameRepresentations(frame);
    }
    else
    {
      m_lazyFrames[m_lastFrameChanged] << frame->time;

      if(hasRepresentations() && (requiresReset(frame) || requiresFocus(frame)))
      {
        waitForDisplay(frame);
      }
    }
  }
  else
  {
    if (!isIdle())
    {
      m_frames.reusePreviousValue(frame->time);
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
bool RepresentationManager::acceptFrame(const FrameCSPtr frame)
{
  return acceptCrosshairChange(frame->crosshair)
      || acceptSceneResolutionChange(frame->resolution)
      || acceptSceneBoundsChange(frame->bounds);
}

//-----------------------------------------------------------------------------
NmVector3 RepresentationManager::currentCrosshair() const
{
  return lastFrame()->crosshair;
}

//-----------------------------------------------------------------------------
NmVector3 RepresentationManager::currentSceneResolution() const
{
  return lastFrame()->resolution;
}

//-----------------------------------------------------------------------------
Bounds RepresentationManager::currentSceneBounds() const
{
  return lastFrame()->bounds;
}

//-----------------------------------------------------------------------------
void RepresentationManager::emitRenderRequest(const GUI::Representations::FrameCSPtr frame)
{
  //qDebug() << debugName() << "Render request for" << frame;
  if((m_frames.isEmpty() && isValid(frame)) || frame->time > m_frames.lastTime())
  {
    m_frames.addValue(frame, frame->time);

    auto it = m_lazyFrames.find(frame->time);
    if(it != m_lazyFrames.end())
    {
      for(auto time: it.value())
      {
        m_frames.reusePreviousValue(time);
      }

      m_lazyFrames.erase(it);
    }

    qDebug() << debugName() << "Actors ready for" << frame << "emit request";
    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::waitForDisplay(const FrameCSPtr frame)
{
  //qDebug() << debugName() << "waiting actors for" << frame;
  Q_ASSERT(m_lastFrameChanged <= frame->time);
  m_lastFrameChanged = frame->time;

  m_status = Status::PENDING_DISPLAY;
}

//-----------------------------------------------------------------------------
void RepresentationManager::idle()
{
  m_status = Status::IDLE;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return lastFrame()->crosshair != crosshair;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return lastFrame()->resolution != resolution;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return lastFrame()->bounds != bounds;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::needsRepresentationUpdate(const FrameCSPtr frame)
{
  return !isValid(lastFrame()) || acceptFrame(frame);
}

//-----------------------------------------------------------------------------
bool RepresentationManager::waitingNewerFrames(TimeStamp t) const
{
  return t < m_lastFrameChanged;
}

//-----------------------------------------------------------------------------
void RepresentationManager::updateRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  onShow(frame);

  if (hasRepresentations())
  {
    waitForDisplay(frame);

  }

  updateFrameRepresentations(frame);
}

//-----------------------------------------------------------------------------
RepresentationPoolSList RepresentationManager::pools() const
{
  // NOTE: default value returned, implement if needed in the subclass.
  return RepresentationPoolSList();
}
