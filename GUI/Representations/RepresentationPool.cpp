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
    disconnect(m_sources, SIGNAL(sourcesAdded(ViewItemAdapterList,TimeStamp)),
               this,      SLOT(onSourcesAdded(ViewItemAdapterList,TimeStamp)));
    disconnect(m_sources, SIGNAL(sourcesRemoved(ViewItemAdapterList,TimeStamp)),
               this,      SLOT(onSourcesRemoved(ViewItemAdapterList, TimeStamp)));
    disconnect(m_sources, SIGNAL(representationsModified(ViewItemAdapterList,TimeStamp)),
               this,      SLOT(onRepresentationModified(ViewItemAdapterList, TimeStamp)));
    disconnect(m_sources, SIGNAL(updateTimeStamp(TimeStamp)),
               this,      SLOT(onTimeStampUpdated(TimeStamp)));

    onSourcesRemoved(m_sources->sources(), m_requestedTimeStamp);
  }

  m_sources = sources;

  if (m_sources)
  {
    connect(m_sources, SIGNAL(sourcesAdded(ViewItemAdapterList, TimeStamp)),
            this,      SLOT(onSourcesAdded(ViewItemAdapterList, TimeStamp)));
    connect(m_sources, SIGNAL(sourcesRemoved(ViewItemAdapterList, TimeStamp)),
            this,      SLOT(onSourcesRemoved(ViewItemAdapterList, TimeStamp)));
    connect(m_sources, SIGNAL(representationsModified(ViewItemAdapterList,TimeStamp)),
            this,      SLOT(onRepresentationModified(ViewItemAdapterList, TimeStamp)));
    connect(m_sources, SIGNAL(updateTimeStamp(TimeStamp)),
            this,      SLOT(onTimeStampUpdated(TimeStamp)));

    onSourcesAdded(m_sources->sources(), m_requestedTimeStamp);
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
    else
    {
      setCrosshairImplementation(point, t);
    }
  }
}

//-----------------------------------------------------------------------------
TimeRange RepresentationPool::readyRange() const
{
  return m_validActors.timeRange();
}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasSources() const
{
  return m_sourcesCount > 0;
}

//-----------------------------------------------------------------------------
RepresentationPipeline::Actors RepresentationPool::actors(TimeStamp t)
{
  return m_validActors.value(t, RepresentationPipeline::Actors());
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidatePreviousActors(TimeStamp t)
{
  m_validActors.invalidatePreviousValues(t);
}

//-----------------------------------------------------------------------------
TimeStamp RepresentationPool::lastUpdateTimeStamp() const
{
  return m_validActors.lastTime();
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
bool RepresentationPool::notHasBeenProcessed(const TimeStamp t) const
{
  return t > m_validActors.lastTime();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onActorsReady(TimeStamp t, RepresentationPipeline::Actors actors)
{
  if (notHasBeenProcessed(t))
  {
    if (actorsChanged())
    {
      m_validActors.addValue(actors, t);

      emit actorsReady(t);
    }
    else
    {
      m_validActors.reusePreviousValue(t);
    }

    emit poolUpdated(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesAdded(ViewItemAdapterList sources, TimeStamp t)
{
  m_sourcesCount += sources.size();

  m_pendingSources << sources;

  if (hasActorsDisplayed())
  {
    processPendingSources();

    invalidateActors();

    invalidateRepresentations(sources, t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesRemoved(ViewItemAdapterList sources, TimeStamp t)
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

    invalidateRepresentations(ViewItemAdapterList(), t);
  }

  Q_ASSERT(m_sourcesCount - sources.size() >= 0);

  m_sourcesCount -= sources.size();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onRepresentationModified(ViewItemAdapterList sources, TimeStamp t)
{
  invalidateActors();

  invalidateRepresentations(sources, t);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onTimeStampUpdated(TimeStamp t)
{
  m_validActors.reusePreviousValue(t);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onRepresentationsInvalidated(ViewItemAdapterPtr item)
{
  ViewItemAdapterList sources;

  sources << item;

  onRepresentationModified(sources, m_validActors.lastTime());
}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasActorsDisplayed() const
{
  return m_numObservers > 0 && hasSources();
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidateActors()
{
  m_validActors.invalidate();

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

  if (m_validActors.isEmpty())
  {
    setCrosshairImplementation(m_crosshair, m_requestedTimeStamp);
  }

  m_pendingSources.clear();
}
