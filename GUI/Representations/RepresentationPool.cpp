/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RepresentationPool.h"
#include <QApplication>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationPool::RepresentationPool()
: m_sources            {nullptr}
, m_numObservers       {0}
, m_requestedTimeStamp {0}
, m_lastUpdateTimeStamp{0}
{
}

//-----------------------------------------------------------------------------
RepresentationPool::~RepresentationPool()
{
  qDebug() << "Destroyed";
}

//-----------------------------------------------------------------------------
void RepresentationPool::setPipelineSources(PipelineSources *sources)
{
  if (m_sources)
  {
    disconnect(m_sources, SIGNAL(sourceAdded(ViewItemAdapterPtr)),
               this,      SLOT(onSourceAdded(ViewItemAdapterPtr)));
    disconnect(m_sources, SIGNAL(sourceRemoved(ViewItemAdapterPtr)),
               this,      SLOT(onSourceRemoved(ViewItemAdapterPtr)));
    disconnect(m_sources, SIGNAL(sourceUpdated(ViewItemAdapterPtr)),
               this,      SLOT(onSourceUpdated(ViewItemAdapterPtr)));
  }

  m_sources = sources;

  if (m_sources)
  {
    connect(m_sources, SIGNAL(sourceAdded(ViewItemAdapterPtr)),
            this,      SLOT(onSourceAdded(ViewItemAdapterPtr)));
    connect(m_sources, SIGNAL(sourceRemoved(ViewItemAdapterPtr)),
            this,      SLOT(onSourceRemoved(ViewItemAdapterPtr)));
    connect(m_sources, SIGNAL(sourceUpdated(ViewItemAdapterPtr)),
            this,      SLOT(onSourceUpdated(ViewItemAdapterPtr)));
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::setSettings(RepresentationPool::SettingsSPtr settings)
{
  m_settings = settings;

  //updateImplementation();?
}

//-----------------------------------------------------------------------------
RepresentationPipeline::Settings RepresentationPool::settings() const
{
  RepresentationPipeline::Settings settings;

  if (m_settings)
  {
    settings = m_settings->pipelineSettings();
  }

  return settings;
}

//-----------------------------------------------------------------------------
void RepresentationPool::setCrosshair(const NmVector3 &point, TimeStamp t)
{
  m_requestedTimeStamp = t;
  m_crosshair          = point;

  setCrosshairImplementation(point, t);
}

//-----------------------------------------------------------------------------
void RepresentationPool::update()
{
  if (isBeingUsed())
  {
    bool needUpdate = m_lastUpdateTimeStamp < m_requestedTimeStamp;

    if (hasPendingSources())
    {
      processPendingSources();

      needUpdate = true;
    }

    if (needUpdate)
    {
      updateImplementation();
    }

    QApplication::sendPostedEvents();
  }
}

//-----------------------------------------------------------------------------
TimeRange RepresentationPool::readyRange() const
{
  return isBeingUsed()?m_frames.keys():TimeRange();
}

//-----------------------------------------------------------------------------
RepresentationPipelineSList RepresentationPool::pipelines(TimeStamp time)
{
//   QList<TimeStamp> timeRange = m_frames.keys();
//
//   for (auto timeStamp : timeRange)
//   {
//     if (timeStamp < time)
//     {
//       m_frames.remove(timeStamp);
//     }
//   }

  return m_frames.value(time, RepresentationPipelineSList());
}

//-----------------------------------------------------------------------------
void RepresentationPool::incrementObservers()
{
  ++m_numObservers;
}

//-----------------------------------------------------------------------------
void RepresentationPool::decrementObservers()
{
  if (m_numObservers == 0) qWarning() << "Unexpected observer decrement";

  --m_numObservers;
}

//-----------------------------------------------------------------------------
bool RepresentationPool::isBeingUsed() const
{
  return m_numObservers > 0;
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationPool::sources() const
{
  return m_sources->sources();
}

//-----------------------------------------------------------------------------
void RepresentationPool::pipelinesReady(TimeStamp time, RepresentationPipelineSList pipelines)
{
  //qDebug() << "Representation ready" << time;
  if (time > m_lastUpdateTimeStamp)
  {
    m_frames[time] = pipelines;

    m_lastUpdateTimeStamp = time;

    //qDebug() << "Representation ready emit" << time;
    emit representationsReady();
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::pipelinesReady2(TimeStamp time, RepresentationPipelineSList pipelines)
{
  if (time > m_lastUpdateTimeStamp)
  {
    m_frames[time] = pipelines;

    m_lastUpdateTimeStamp = time;

    qDebug() << "Representation ready emit" << time;
    emit representationsReady();
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidatePipeline(TimeStamp time)
{
  m_frames.remove(time);
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidateRepresentations()
{
  m_frames.clear();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourceAdded(ViewItemAdapterPtr source)
{
    m_pendingSources << source;

    update();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesAdded(ViewItemAdapterList sources)
{

}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourceRemoved(ViewItemAdapterPtr source)
{

}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesRemoved(ViewItemAdapterList sources)
{

}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourceUpdated(ViewItemAdapterPtr source)
{

}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesUpdated(ViewItemAdapterList sources)
{

}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasPendingSources() const
{
  return !m_pendingSources.isEmpty();
}

//-----------------------------------------------------------------------------
void RepresentationPool::processPendingSources()
{
  for (auto source : m_pendingSources)
  {
    addRepresentationPipeline(source);
  }

  setCrosshairImplementation(m_crosshair, m_requestedTimeStamp);

  m_pendingSources.clear();
}
