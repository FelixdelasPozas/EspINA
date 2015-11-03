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
#include "Frame.h"
#include <GUI/Model/Utils/ModelUtils.h>

// VTK
#include <vtkProp.h>

// Qt
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//-----------------------------------------------------------------------------
RepresentationState PoolSettings::settings()
{
  RepresentationState state;

  state.apply(m_state);

  return state;
}

//-----------------------------------------------------------------------------
RepresentationPool::RepresentationPool(const ItemAdapter::Type &type)
: m_type        {type}
, m_sources     {nullptr}
, m_settings    {new PoolSettings()}
, m_numObservers{0}
, m_sourcesCount{0}
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
    if (ItemAdapter::Type::CHANNEL == m_type)
    {
      disconnect(m_sources, SIGNAL(stacksAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(stacksRemoved(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesRemoved(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(stacksInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(stackColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourceColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
    }
    else
    {
      disconnect(m_sources, SIGNAL(segmentationsAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(segmentationsRemoved(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesRemoved(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(segmentationsInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourcesInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      disconnect(m_sources, SIGNAL(segmentationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
                 this,      SLOT(onSourceColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
    }

    removeSources(m_sources->sources(m_type));
  }

  m_sources = sources;

  if (m_sources)
  {
    if (ItemAdapter::Type::CHANNEL == m_type)
    {
      connect(m_sources, SIGNAL(stacksAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(stacksRemoved(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesRemoved(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(stacksInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(stackColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourceColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
    }
    else
    {
      connect(m_sources, SIGNAL(segmentationsAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesAdded(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(segmentationsRemoved(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesRemoved(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(segmentationsInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourcesInvalidated(ViewItemAdapterList, GUI::Representations::FrameCSPtr)));
      connect(m_sources, SIGNAL(segmentationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
              this,      SLOT(onSourceColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
    }

    addSources(m_sources->sources(m_type));
  }
  //NOTE: vamos a ver si podemos dejar que lo haga otro metodo>> updateRepresentationsAt(t, m_sources->sources());
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationPool::sources() const
{
  return m_sources->sources(m_type);
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
void RepresentationPool::updatePipelines(const GUI::Representations::FrameCSPtr frame)
{
  if (notHasBeenProcessed(frame->time))
  {
    processPendingSources();

    updatePipelinesImplementation(frame);
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
  return m_sourcesCount > 0 || hasPendingSources();
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
void RepresentationPool::reuseRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  if (!m_validActors.isEmpty())
  {
    m_validActors.reusePreviousValue(frame->time);

    emit actorsReady(frame);
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
  if (m_numObservers == 0)
  {
    qWarning() << "Unexpected observer decrement";
  }
  else
  {
    --m_numObservers;
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::invalidateRepresentations(ViewItemAdapterList items, GUI::Representations::FrameCSPtr frame)
{
  onSourcesInvalidated(items, frame);
}

//-----------------------------------------------------------------------------
bool RepresentationPool::notHasBeenProcessed(const TimeStamp t) const
{
  return m_validActors.isEmpty() || t > m_validActors.lastTime();
}

//-----------------------------------------------------------------------------
void RepresentationPool::onActorsReady(const GUI::Representations::FrameCSPtr frame, RepresentationPipeline::Actors actors)
{
  if (notHasBeenProcessed(frame->time))
  {
    if (actorsChanged(actors))
    {
      m_validActors.addValue(actors, frame->time);
    }
    else
    {
      m_validActors.reusePreviousValue(frame->time);
    }

    emit actorsReady(frame);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesAdded(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  addSources(sources);

  if (isEnabled())
  {
    processPendingSources();

    updateRepresentationsAt(frame, sources);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesRemoved(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  if (removeSources(sources))
  {
    updateRepresentationsAt(frame);
  }

  if(m_sourcesCount == 0)
  {
    m_validActors.invalidate();

    emit actorsInvalidated(frame);

    onActorsReady(frame, RepresentationPipeline::Actors());
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourcesInvalidated(ViewItemAdapterList sources,  const GUI::Representations::FrameCSPtr frame)
{
  updateRepresentationsAt(frame, sources);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSourceColorsInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  qDebug() << "RepresentationPool: colors invalidated" << frame->time;
  updateRepresentationColorsAt(frame, sources);
}

//-----------------------------------------------------------------------------
void RepresentationPool::onSettingsModified()
{
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
void RepresentationPool::updateRepresentationsAt(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
{
  m_validActors.invalidate();

  if (isEnabled())
  {
    emit actorsInvalidated(frame);

    updateRepresentationsAtImlementation(frame, modifiedItems);
  }
}

//-----------------------------------------------------------------------------
void RepresentationPool::updateRepresentationColorsAt(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
{
  m_validActors.invalidate();

  if (isEnabled())
  {
    emit actorsInvalidated(frame);

    updateRepresentationColorsAtImlementation(frame, modifiedItems);
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
