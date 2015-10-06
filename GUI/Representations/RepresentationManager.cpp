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

  if (frame->isValid())
  {
    if (m_isActive)
    {
      updateRepresentations(frame);
    }
  }
  else
  {
    onShow(frame->time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show(const GUI::Representations::FrameCSPtr frame)
{
  m_isActive = true;

  if (m_view)
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
    onHide(frame->time);

    //qDebug() << debugName() << "Requested hide" << t;

    if(hasRepresentations())
    {
      waitForDisplay();

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
//   TimeRange range;
//
//   if (isActive())
//   {
//     range = readyRangeImplementation();
//   }
//   else
//   {
//     range << m_latestFrame->time;
//   }
//
//   return range;
}

//-----------------------------------------------------------------------------
FrameCSPtr RepresentationManager::frame(TimeStamp t) const
{
  auto result = std::make_shared<Frame>();

  if (!m_frames.isEmpty() && t <= m_frames.lastTime())
  {
    auto value = m_frames.value(t);

    result->time = t;
    result->crosshair = value->crosshair;
    result->resolution = value->resolution;
    result->bounds = value->bounds;
    result->focus = value->focus;
    result->reset = value->reset;
  }

  return result;
}

//-----------------------------------------------------------------------------
FrameCSPtr RepresentationManager::lastFrame() const
{
  return frame(m_frames.lastTime());
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(const GUI::Representations::FrameCSPtr frame)
{
  Q_ASSERT(m_view);

  if (isActive())
  {
    //qDebug() << debugName() << "Display actors at" << t;
    displayRepresentations(frame->time);
    m_frames.invalidatePreviousValues(frame->time);
  }
  else
  {
    //qDebug() << debugName() << "Hide frame" << m_lastRenderRequestTime << "at" << t;
    hideRepresentations(m_frames.lastTime());
  }

  if (!hasNewerFrames(frame->time))
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
void RepresentationManager::onFrameChanged(const FrameCSPtr frame)
{
  if (!frame->isValid()) return;

  if (needsRepresentationUpdate(frame))
  {
    if (isActive())
    {
      if(hasRepresentations())
      {
        waitForDisplay();
      }

      updateFrameRepresentations(frame);
    }
  }
  else if (!isIdle())
  {
    m_frames.reusePreviousValue(frame->time);
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
  qDebug() << debugName() << "Requested to emit renderRequested at" << frame->time;
  if(m_frames.isEmpty() || frame->time > m_frames.lastTime())
  {
    m_frames.addValue(frame, frame->time);

    qDebug() << debugName() << "Emit renderRequested at" << frame->time;
    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::invalidateRepresentations()
{
  qDebug() << debugName() << "last render timestamp invalid";
  m_frames.invalidate();
}

//-----------------------------------------------------------------------------
void RepresentationManager::waitForDisplay()
{
  qDebug() << debugName() << "pending display";
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
  auto currentFrame = lastFrame();

  return !currentFrame->isValid()
       || acceptFrame(frame)
       || frame->focus != currentFrame->focus
       || frame->reset != currentFrame->reset;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::hasNewerFrames(TimeStamp t) const
{
  return t < m_frames.lastTime();
}

//-----------------------------------------------------------------------------
void RepresentationManager::updateRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  onShow(frame->time);

  if (hasRepresentations())
  {
    waitForDisplay();
  }

  updateFrameRepresentations(frame);
}

//-----------------------------------------------------------------------------
RepresentationPoolSList RepresentationManager::pools() const
{
  // NOTE: default value returned, implement if needed in the subclass.
  return RepresentationPoolSList();
}

//-----------------------------------------------------------------------------
