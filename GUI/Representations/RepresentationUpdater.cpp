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

#include "RepresentationUpdater.h"
#include <vtkProp.h>
#include <QCoreApplication>

using namespace ESPINA;

//----------------------------------------------------------------------------
RepresentationUpdater::RepresentationUpdater(SchedulerSPtr scheduler,
                                             RepresentationPipelineSPtr pipeline)
: Task            {scheduler}
, m_timeStamp     {0}
, m_timeStampValid{false}
, m_pipeline      {pipeline}
, m_updateList    {&m_sources}
{
  setHidden(true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::addSource(ViewItemAdapterPtr item)
{
  QMutexLocker lock(&m_mutex);

  if(!m_sources.contains(item))
  {
    m_sources << item;
  }
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removeSource(ViewItemAdapterPtr item)
{
  QMutexLocker lock(&m_mutex);

  if(m_actors.keys().contains(item))
  {
    m_actors.remove(item);
  }

  if(m_sources.contains(item))
  {
    m_sources.removeOne(item);
  }

  if(m_requestedSources.contains(item))
  {
    m_requestedSources.removeOne(item);
  }
}

//----------------------------------------------------------------------------
bool RepresentationUpdater::isEmpty() const
{
  QMutexLocker lock(&m_mutex);

  return m_sources.isEmpty();
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setCrosshair(const NmVector3 &point)
{
  QMutexLocker lock(&m_mutex);

  setCrosshairPoint(point, m_settings);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setResolution(const NmVector3 &resolution)
{
  // TODO 2015-04-20 apply resolution change
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setSettings(const RepresentationState &settings)
{
  QMutexLocker lock(&m_mutex);

  if (hasCrosshairPoint(m_settings))
  {
    auto point = crosshairPoint(m_settings);

    m_settings = settings;

    setCrosshairPoint(point, m_settings);
  }
  else
  {
    m_settings = settings;
  }

  restart();
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setUpdateList(ViewItemAdapterList sources)
{
  QMutexLocker lock(&m_mutex);

  for(auto item: sources)
  {
    if(!m_requestedSources.contains(item))
    {
      m_requestedSources << item;
    }
  }

  m_updateList = &m_requestedSources;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setTimeStamp(TimeStamp time)
{
  QMutexLocker lock(&m_mutex);

  m_timeStamp      = time;
  m_timeStampValid = true;
}

//----------------------------------------------------------------------------
TimeStamp RepresentationUpdater::timeStamp() const
{
  return m_timeStamp;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::invalidate()
{
  m_timeStampValid = false;
}

//----------------------------------------------------------------------------
bool RepresentationUpdater::hasValidTimeStamp() const
{
  return m_timeStampValid;
}

//----------------------------------------------------------------------------
ViewItemAdapterList RepresentationUpdater::pick(const NmVector3 &point, vtkProp *actor) const
{
  QMutexLocker lock(&m_mutex);

  ViewItemAdapterList pickedItems;

  if (actor)
  {
    auto foundItem = findActorItem(actor);

    if (foundItem && m_pipeline->pick(foundItem, point))
    {
      pickedItems << foundItem;
    }
  }
  else
  {
    for (auto item : m_actors.keys())
    {
      if (m_pipeline->pick(item, point))
      {
        pickedItems << item;
      }
    }
  }

  return pickedItems;
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors RepresentationUpdater::actors() const
{
  QMutexLocker lock(&m_mutex);

  return m_actors;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::run()
{
  QMutexLocker lock(&m_mutex);

  //qDebug() << "Task" << description() << "running" << " - " << this;

  // Local copy needed to prevent condition race on same TimeStamp
  // (usually due to invalidation view item representations)

  auto updateList = *m_updateList;
  m_updateList    = &m_sources;
  auto size = updateList.size();

  int i = 0;

  auto it = updateList.begin();
  while (canExecute() && it != updateList.end())
  {
    auto  item     = *it;
    auto  pipeline = sourcePipeline(item);

    auto state  = pipeline->representationState(item, m_settings);
    auto actors = pipeline->createActors(item, state);

    m_actors[item]  = actors;

    ++it;
    ++i;

    reportProgress((i/static_cast<double>(size))*100);
  }

  if (hasValidTimeStamp() && canExecute())
  {
    m_requestedSources.clear();
    emit actorsReady(timeStamp(), m_actors);
  }
}

//----------------------------------------------------------------------------
RepresentationPipelineSPtr RepresentationUpdater::sourcePipeline(ViewItemAdapterPtr item) const
{
  auto pipeline = item->temporalRepresentation();

  if (!pipeline || pipeline->type() != m_pipeline->type())
  {
    pipeline = m_pipeline;
  }

  return pipeline;
}

//----------------------------------------------------------------------------
ViewItemAdapterPtr RepresentationUpdater::findActorItem(vtkProp *actor) const
{
  ViewItemAdapterPtr item = nullptr;

  auto it = m_actors.begin();
  while (it != m_actors.end() && !item)
  {
    for (auto itemActor : it.value())
    {
      if (itemActor.GetPointer() == actor)
      {
        item = it.key();
        break;
      }
    }

    ++it;
  }

  return item;
}
