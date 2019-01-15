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
, m_lastInvalidationFrame{Timer::INVALID_TIME_STAMP}
{
}

//-----------------------------------------------------------------------------
RepresentationManager::~RepresentationManager()
{
  emit terminated(this);

  if(m_view) shutdown();
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

  if (m_view && isActive())
  {
    onShow(frame);

    if (hasRepresentations())
    {
      updateRepresentations(frame);
    }
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::show(const GUI::Representations::FrameCSPtr frame)
{
  m_isActive = true;

  if(m_view)
  {
    onShow(frame);

    if (isActive() && hasRepresentations())
    {
      updateRepresentations(frame);
    }
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
  if (!m_frames.isEmpty() && t <= m_frames.lastTime())
  {
    return m_frames.value(t, Frame::InvalidFrame());
  }

  return Frame::InvalidFrame();
}

//-----------------------------------------------------------------------------
FrameCSPtr RepresentationManager::lastFrame() const
{
  return frame(m_frames.lastTime());
}

//-----------------------------------------------------------------------------
void RepresentationManager::display(TimeStamp time)
{
  if(!m_view) return;

  if(m_frames.value(time, Frame::InvalidFrame()) == Frame::InvalidFrame())
  {
    return;
  }

  if (isActive())
  {
    // qDebug() << "\t" << debugName() << "Display actors at" << time << "corresponding frame" << m_frames.value(time)->time;
    displayRepresentations(m_frames.value(time));

    m_frames.invalidatePreviousValues(time);
  }
  else
  {
    // qDebug() << "\t" << debugName() << "Hide frame at" << time << lastFrame();
    auto last = lastFrame();
    hideRepresentations(last);
    m_frames.invalidatePreviousValues(last->time);
  }

  if (!waitingNewerFrames(time))
  {
    // qDebug() << "\t" << debugName() << "Displayed last frame" << time << "going idle";
    idle();
  }
  else
  {
    // qDebug() << "\t" << debugName() << "Waiting from frame" << m_lastFrameChanged;
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

  m_childs << child.get();

  connect(child.get(), SIGNAL(terminated(RepresentationManager *)),
          this,        SLOT(onChildTerminated(RepresentationManager *)));

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
  if (!isValid(frame) || frame->time <= m_lastFrameChanged) return;

//   qDebug() << debugName() << "received frame" << frame->time << "status" << ((m_status == Status::IDLE) ? "Idle" : QString("Waiting %1").arg(m_lastFrameChanged)) << "last time" << m_frames.lastTime();
  if (isActive())
  {
    if (needsRepresentationUpdate(frame))
    {
      if(hasRepresentations())
      {
        waitForDisplay(frame);
      }

      updateFrameRepresentations(frame);
//       qDebug() << debugName() << "processed frame" << frame->time << "status" << ((m_status == Status::IDLE) ? "Idle" : QString("Waiting %1").arg(m_lastFrameChanged)) << path;
      return;
    }
    else
    {
      if(acceptInvalidationFrame(frame) && hasRepresentations())
      {
        waitForDisplay(frame);
//         qDebug() << debugName() << "processed frame" << frame->time << "status" << ((m_status == Status::IDLE) ? "Idle" : QString("Waiting %1").arg(m_lastFrameChanged)) << path;
        return;
      }
    }
  }

  if (!isIdle())
  {
    reuseTimeValue(frame->time);
  }

//   qDebug() << debugName() << "processed frame" << frame->time << "status" << ((m_status == Status::IDLE) ? "Idle" : QString("Waiting %1").arg(m_lastFrameChanged)) << path;
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
    m_flags &= ~flag;
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptFrame(const FrameCSPtr frame)
{
  return acceptCrosshairChange(frame->crosshair)        ||
         acceptSceneResolutionChange(frame->resolution) ||
         acceptSceneBoundsChange(frame->bounds);
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
//  qDebug() << debugName() << "ask renderrequest" << frame->time << "last inv" << m_lastInvalidationFrame;
  if(frame->time < m_lastInvalidationFrame) return;

  if((m_frames.isEmpty() && isValid(frame)) || frame->time >= m_frames.lastTime())
  {
    m_frames.addValue(frame, frame->time);

    if(m_lazyFrames.keys().contains(frame->time))
    {
      m_frames.reusePreviousValue(m_lazyFrames.value(frame->time));

      m_lazyFrames.remove(frame->time);
    }

//    qDebug() << debugName() << "emit" << frame->time;
    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::waitForDisplay(const FrameCSPtr frame)
{
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
  return !isValid(lastFrame()) || lastFrame()->crosshair != crosshair;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return !isValid(lastFrame()) || lastFrame()->resolution != resolution;
}

//-----------------------------------------------------------------------------
bool RepresentationManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return !isValid(lastFrame()) || lastFrame()->bounds != bounds;
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

//-----------------------------------------------------------------------------
bool GUI::Representations::invalidatesRepresentations(GUI::Representations::FrameCSPtr frame, ItemAdapter::Type type)
{
  if(ItemAdapter::Type::SEGMENTATION == type)
  {
    return invalidatesSegmentations(frame);
  }
  else
  {
    if(ItemAdapter::Type::CHANNEL == type)
    {
      return invalidatesChannels(frame);
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void RepresentationManager::reuseTimeValue(TimeStamp t)
{
  Q_ASSERT(!isIdle());

  // current actor already computed in the pool.
  if(m_frames.lastTime() == m_lastFrameChanged)
  {
    m_frames.reusePreviousValue(t);
    m_lastFrameChanged = t;
  }
  else
  {
    m_lazyFrames.insert(m_lastFrameChanged, t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::invalidateFrames(const FrameCSPtr frame)
{
  // qDebug() << debugName() << "invalidates frames on" << frame->time;
  m_lastInvalidationFrame = frame->time;
  m_frames.invalidate();
}

//-----------------------------------------------------------------------------
void RepresentationManager::shutdown()
{
  setView(nullptr, Frame::InvalidFrame());

  for(auto child: m_childs)
  {
    disconnect(child, SIGNAL(terminated(RepresentationManager *)),
               this,  SLOT(onChildTerminated(RepresentationManager *)));
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager::onChildTerminated(RepresentationManager *sender)
{
  auto manager = dynamic_cast<RepresentationManager *>(sender);

  if(manager)
  {
    auto equalOp = [&manager] (const RepresentationManager *child) { return manager == child; };
    auto it = std::find_if(m_childs.begin(), m_childs.end(), equalOp);
    if(it != m_childs.end())
    {
      m_childs.removeOne(*it);
      return;
    }
  }

  qWarning() << __FILE__ << __LINE__ << "Received object but couldn't identify it.";
}
