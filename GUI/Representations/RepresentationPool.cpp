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

// ESPINA
#include "RepresentationPool.h"

// VTK
#include <vtkProp.h>

// Qt
#include <QApplication>

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationState PoolSettings::settings()
{
  RepresentationState state;

  state.apply(m_state);

  return state;
}

//-----------------------------------------------------------------------------
RepresentationPool::RepresentationPool()
: m_sources            {nullptr}
, m_settings           {new PoolSettings()}
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
  auto t = sources->invalidator().timer().increment();

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

    removeSources(m_sources->sources());
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

    addSources(m_sources->sources());
  }

  updateRepresentationsAt(t, m_sources->sources());
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationPool::sources() const
{
  return m_sources->sources();
}

//-----------------------------------------------------------------------------
void RepresentationPool::setSettings(PoolSettingsSPtr settings)
{
  if(m_settings)
  {
    disconnect(m_settings.get(), SIGNAL(modified()),
               this,             SLOT(onSettingsModified()));
  }

  m_settings = settings;

  if(m_settings)
  {
    connect(m_settings.get(), SIGNAL(modified()),
            this,             SLOT(onSettingsModified()));

    m_poolState.apply(settings->settings());

    applySettings(m_poolState);
  }
}

//-----------------------------------------------------------------------------
RepresentationState RepresentationPool::settings() const
{
  return m_settings->settings();
}

//-----------------------------------------------------------------------------
void RepresentationPool::setCrosshair(const NmVector3 &crosshair, TimeStamp t)
{
  m_crosshair = crosshair;

  if (notHasBeenProcessed(t))
  {
    setCrosshairImplementation(crosshair, t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::setSceneResolution(const NmVector3 &resolution, TimeStamp t)
{
  m_resolution = resolution;

  setSceneResolutionImplementation(resolution, t);
}

//-----------------------------------------------------------------------------
void RepresentationPool::updatePipelines(const NmVector3 &crosshair, const NmVector3 &resolution, TimeStamp t)
{
  if (notHasBeenProcessed(t))
  {
    processPendingSources();

    updatePipelinesImplementation(crosshair, resolution, t);
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
void RepresentationPool::reuseRepresentations(TimeStamp t)
{
  if (!m_validActors.isEmpty())
  {
    m_validActors.reusePreviousValue(t);

    emit actorsReady(t);
  }
}

//-----------------------------------------------------------------------------
TimeStamp RepresentationPool::lastUpdateTimeStamp() const
{
  return m_validActors.lastTime();
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
void RepresentationPool::invalidateRepresentations(ViewItemAdapterList items, TimeStamp t)
{
  onRepresentationModified(items, t);
}

//-----------------------------------------------------------------------------
bool RepresentationPool::notHasBeenProcessed(const TimeStamp t) const
{
  return m_validActors.isEmpty() || t > m_validActors.lastTime();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onActorsReady(TimeStamp t, RepresentationPipeline::Actors actors)
{
  if (notHasBeenProcessed(t))
  {
    if (actorsChanged(actors))
    {
      m_validActors.addValue(actors, t);
    }
    else
    {
      m_validActors.reusePreviousValue(t);
    }
    emit actorsReady(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesAdded(ViewItemAdapterList sources, TimeStamp t)
{
  addSources(sources);

  if (isEnabled())
  {
    processPendingSources();

    updateRepresentationsAt(t, sources);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesRemoved(ViewItemAdapterList sources, TimeStamp t)
{
  if (removeSources(sources))
  {
    updateRepresentationsAt(t);
  }

  if(m_sourcesCount == 0)
  {
    m_validActors.invalidate();

    emit actorsInvalidated();

    onActorsReady(t, RepresentationPipeline::Actors());
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onRepresentationModified(ViewItemAdapterList sources, TimeStamp t)
{
  updateRepresentationsAt(t, sources);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onTimeStampUpdated(TimeStamp t)
{
  if(!m_validActors.isEmpty())
  {
    m_validActors.reusePreviousValue(t);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSettingsModified()
{
  emit actorsInvalidated();

  m_poolState.apply(m_settings->settings());

  applySettings(m_poolState);
}

//-----------------------------------------------------------------------------
bool RepresentationPool::isEnabled() const
{
  return m_numObservers > 0 && hasSources();
}

//-----------------------------------------------------------------------------
bool RepresentationPool::actorsChanged(const RepresentationPipeline::Actors &actors) const
{
  return m_validActors.isEmpty() || m_validActors.last() != actors;
}

//-----------------------------------------------------------------------------
void RepresentationPool::updateRepresentationsAt(TimeStamp t, ViewItemAdapterList modifiedItems)
{
  m_validActors.invalidate();

  if (isEnabled())
  {
    emit actorsInvalidated();

    updateRepresentationsImlementationAt(t, modifiedItems);
  }
}

//-----------------------------------------------------------------------------
bool RepresentationPool::hasPendingSources() const
{
  return !m_pendingSources.isEmpty();
}

//-----------------------------------------------------------------------------
void RepresentationPool::addSources(ViewItemAdapterList sources)
{
  m_sourcesCount += sources.size();

  m_pendingSources << sources;
}

//-----------------------------------------------------------------------------
bool RepresentationPool::removeSources(ViewItemAdapterList sources)
{
  bool removed = false;

  for (auto source : sources)
  {
    if (m_pendingSources.contains(source))
    {
      m_pendingSources.removeOne(source);
    }
    else
    {
      removeRepresentationPipeline(source);
      removed = true;
    }
  }

  m_sourcesCount -= sources.size();

  return removed;
}

//-----------------------------------------------------------------------------
void RepresentationPool::processPendingSources()
{
  for (auto source : m_pendingSources)
  {
    addRepresentationPipeline(source);
  }

  m_pendingSources.clear();
}
