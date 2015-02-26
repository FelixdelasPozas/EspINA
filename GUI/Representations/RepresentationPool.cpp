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
RepresentationState RepresentationPool::Settings::poolSettings()
{
  auto state = poolSettingsImplementation();

  state.apply(m_state);

  return state;
}

//-----------------------------------------------------------------------------
RepresentationState RepresentationPool::Settings::poolSettingsImplementation()
{
  return RepresentationState();
}

//-----------------------------------------------------------------------------
RepresentationPool::RepresentationPool()
: m_sources            {nullptr}
, m_settings           {new Settings()}
, m_requestedTimeStamp {1}
, m_lastUpdateTimeStamp{0}
, m_numObservers       {0}
, m_sourcesCount       {0}
{
}

//-----------------------------------------------------------------------------
RepresentationPool::~RepresentationPool()
{
}

//-----------------------------------------------------------------------------
void RepresentationPool::setPipelineSources(PipelineSources *sources)
{
  if (m_sources)
  {
    disconnect(m_sources, SIGNAL(sourcesAdded(ViewItemAdapterList)),
               this,      SLOT(onSourcesAdded(ViewItemAdapterList)));
    disconnect(m_sources, SIGNAL(sourcesRemoved(ViewItemAdapterList)),
               this,      SLOT(onSourcesRemoved(ViewItemAdapterList)));
    disconnect(m_sources, SIGNAL(sourcesUpdated(ViewItemAdapterList)),
               this,      SLOT(onSourcesUpdated(ViewItemAdapterList)));
  }

  m_sources = sources;

  if (m_sources)
  {
    connect(m_sources, SIGNAL(sourcesAdded(ViewItemAdapterList)),
            this,      SLOT(onSourcesAdded(ViewItemAdapterList)));
    connect(m_sources, SIGNAL(sourcesRemoved(ViewItemAdapterList)),
            this,      SLOT(onSourcesRemoved(ViewItemAdapterList)));
    connect(m_sources, SIGNAL(sourcesUpdated(ViewItemAdapterList)),
            this,      SLOT(onSourcesUpdated(ViewItemAdapterList)));
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::setSettings(RepresentationPool::SettingsSPtr settings)
{
  m_settings = settings;

  m_poolState.apply(settings->poolSettings());

  if (hasActorsDisplayed())
  {
    onSettingsChanged(m_poolState);
  }
}

//-----------------------------------------------------------------------------
RepresentationState RepresentationPool::settings() const
{
  return m_settings->poolSettings();
}

//-----------------------------------------------------------------------------
void RepresentationPool::setCrosshair(const NmVector3 &point, TimeStamp t)
{
  m_requestedTimeStamp = t;
  m_crosshair          = point;

  if (hasActorsDisplayed())
  {
    if (hasPendingSources())
    {
      processPendingSources();
    }

    setCrosshairImplementation(point, t);
  }
}

//-----------------------------------------------------------------------------
TimeRange RepresentationPool::readyRange() const
{
  TimeRange range;

  if (!m_validActorsTimes.isEmpty())
  {
    for (TimeStamp i = m_validActorsTimes.first(); i <= m_lastUpdateTimeStamp; ++i)
    {
      range << i;
    }
  }

  return range;
}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasSources() const
{
  return m_sourcesCount > 0;
}

//-----------------------------------------------------------------------------
RepresentationPipeline::Actors RepresentationPool::actors(TimeStamp time)
{
  int  i     = 1;
  bool found = false;

  Q_ASSERT(!m_validActorsTimes.isEmpty());

  TimeStamp validTime = m_validActorsTimes.first();

  while (!found && i < m_validActorsTimes.size())
  {
    auto nextTime = m_validActorsTimes[i];

    found = nextTime > time;

    if (!found)
    {
      validTime = nextTime;
    }

    ++i;
  }

  return m_actors.value(validTime, RepresentationPipeline::Actors());
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidatePreviousActors(TimeStamp time)
{
  int  i     = 1;
  bool found = false;

  if (m_validActorsTimes.isEmpty()) return;

  auto validTime   = m_validActorsTimes.first();
  auto validActors = m_actors[validTime];

  while (!found && i < m_validActorsTimes.size())
  {
    auto nextTime = m_validActorsTimes[i];

    found = nextTime > time;

    if (!found)
    {
      m_actors.remove(validTime);

      validTime   = nextTime;
      validActors = m_actors[nextTime];

      ++i;
    }
  }

  while (i > 1) // remove invalid timestamps except one to be replaced
  {
    m_validActorsTimes.removeFirst();
    --i;
  }

  m_actors[time]        = validActors;
  m_validActorsTimes[0] = time;
}


//-----------------------------------------------------------------------------
TimeStamp RepresentationPool::lastUpdateTimeStamp() const
{
  return m_lastUpdateTimeStamp;
}

//-----------------------------------------------------------------------------
void RepresentationPool::incrementObservers()
{
  if(m_numObservers == 0)
  {
    onSettingsChanged(m_poolState);
  }

  ++m_numObservers;

}

//-----------------------------------------------------------------------------
void RepresentationPool::decrementObservers()
{
  if (m_numObservers == 0) qWarning() << "Unexpected observer decrement";

  --m_numObservers;
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidate()
{
  if (hasActorsDisplayed())
  {
    invalidateActors();

    invalidateImplementation();
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationPool::sources() const
{
  return m_sources->sources();
}

//-----------------------------------------------------------------------------
bool RepresentationPool::notHasBeenProcessed(const TimeStamp time) const
{
  return time > m_lastUpdateTimeStamp;
}

//-----------------------------------------------------------------------------
void RepresentationPool::onActorsReady(TimeStamp time, RepresentationPipeline::Actors actors)
{
  if (notHasBeenProcessed(time))
  {
    m_lastUpdateTimeStamp = time;

    if (actorsChanged())
    {
      m_actors[time] = actors;
      m_validActorsTimes << time;

      emit actorsReady(time);
    }

    emit poolUpdated(time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesAdded(ViewItemAdapterList sources)
{
  m_sourcesCount += sources.size();

  m_pendingSources << sources;

  if (hasActorsDisplayed())
  {
    processPendingSources();

    invalidateActors();

    invalidateRepresentations(sources);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesRemoved(ViewItemAdapterList sources)
{
  bool invalidate = false;

  for (auto source : sources)
  {
    if (m_pendingSources.contains(source))
    {
      m_pendingSources.removeOne(source);
    }
    else
    {
      removeRepresentationPipeline(source);
      invalidate = true;
    }
  }

  if (invalidate)
  {
    invalidateActors();

    invalidateRepresentations(ViewItemAdapterList());
  }

  m_sourcesCount -= sources.size();

  Q_ASSERT(m_sourcesCount >= 0);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesUpdated(ViewItemAdapterList sources)
{
  invalidateActors();

  invalidateRepresentations(sources);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onRepresentationsInvalidated(ViewItemAdapterPtr item)
{
  ViewItemAdapterList sources;

  sources << item;

  onSourcesUpdated(sources);
}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasActorsDisplayed() const
{
  return m_numObservers > 0 && hasSources();
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidateActors()
{
  m_actors.clear();
  m_validActorsTimes.clear();
  m_lastUpdateTimeStamp = 0;

  emit actorsInvalidated();
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

    connect(source, SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
            this,   SLOT(onRepresentationsInvalidated(ViewItemAdapterPtr)));
  }

  if (m_validActorsTimes.isEmpty())
  {
    setCrosshairImplementation(m_crosshair, m_requestedTimeStamp);
  }


  m_pendingSources.clear();
}
