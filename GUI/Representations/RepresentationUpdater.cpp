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
#include "RepresentationUpdater.h"
#include "Frame.h"

// VTK
#include <vtkProp.h>

// Qt
#include <QCoreApplication>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//----------------------------------------------------------------------------
RepresentationUpdater::RepresentationUpdater(SchedulerSPtr scheduler,
                                             RepresentationPipelineSPtr pipeline)
: Task         {scheduler}
, m_frame      {Frame::InvalidFrame()}
, m_pipeline   {pipeline}
, m_updateList {&m_sources}
, m_actors     {std::make_shared<RepresentationPipeline::ActorsData>()}
{
  setHidden(true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::addSource(ViewItemAdapterPtr item)
{
  QMutexLocker lock(&m_mutex);

  addUpdateRequest(m_sources, item, true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removeSource(ViewItemAdapterPtr item)
{
  QMutexLocker lock(&m_mutex);

  {
    QMutexLocker actorsLock(&m_actors->lock);

    if(m_actors->actors.keys().contains(item))
    {
      m_actors->actors.remove(item);
    }
  }

  removeUpdateRequest(m_sources, item);
  removeUpdateRequest(m_requestedSources, item);
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
void RepresentationUpdater::updateRepresentations(ViewItemAdapterList sources)
{
  updateRepresentations(sources, true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::updateRepresentationColors(const ViewItemAdapterList& sources)
{
  updateRepresentations(sources, false);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setFrame(const GUI::Representations::FrameCSPtr frame)
{
  QMutexLocker lock(&m_mutex);

  m_frame = frame;
}

//----------------------------------------------------------------------------
FrameCSPtr RepresentationUpdater::frame() const
{
  return m_frame;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::invalidate()
{
  m_frame = Frame::InvalidFrame();
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
    QMutexLocker actorsLock(&m_actors->lock);

    for (auto item : m_actors->actors.keys())
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

  QMutexLocker actorsLock(&m_actors->lock);

  return m_actors;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::run()
{
  QMutexLocker lock(&m_mutex);

  // Local copy needed to prevent condition race on same frame
  // (usually due to invalidation view item representations)
  auto updateList = *m_updateList;
  updateList.detach();

  m_updateList    = &m_sources;
  m_requestedSources.clear();

  auto it   = updateList.begin();
  auto size = updateList.size();

  {
    QMutexLocker actorsLock(&m_actors->lock);

    int i = 0;
    while (canExecute() && it != updateList.end())
    {
      auto item     = it->first;
      auto pipeline = sourcePipeline(item);

      auto state = pipeline->representationState(item, m_settings);

      if (it->second)
      {
        m_actors->actors[item] = pipeline->createActors(item, state);
      }

      pipeline->updateColors(m_actors->actors[item], item, state);

      ++it;
      ++i;

      reportProgress((i/static_cast<double>(size))*100);
    }
  }

  if (isValid(m_frame) && canExecute())
  {
    emit actorsReady(m_frame, m_actors);
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

  QMutexLocker lock(&m_actors->lock);

  auto it = m_actors->actors.begin();
  while (it != m_actors->actors.end() && !item)
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

//----------------------------------------------------------------------------
void RepresentationUpdater::updateRepresentations(ViewItemAdapterList sources, const bool createActors)
{
  QMutexLocker lock(&m_mutex);

  ViewItemAdapterList sourceItems;
  for(auto source: m_sources)
  {
    sourceItems << source.first;
  }

  for(auto item: sources)
  {
    if(!sourceItems.contains(item)) continue;

    addUpdateRequest(m_requestedSources, item, createActors);
  }

  m_updateList = &m_requestedSources;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::addUpdateRequest(UpdateRequestList& list,
                                             ViewItemAdapterPtr item,
                                             const bool createActors)
{
  for (auto it = list.begin(); it != list.end(); ++it)
  {
    if (it->first == item)
    {
      it->second |= createActors;

      return;
    }
  }

  list << UpdateRequest(item, createActors);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removeUpdateRequest(UpdateRequestList& list,
                                                ViewItemAdapterPtr item)
{
  int  i     = 0;
  bool found = false;

  while (!found && i < list.size())
  {
    if (list[i].first == item)
    {
      found = true;
    }
    else
    {
      ++i;
    }
  }

  if (found)
  {
    list.removeAt(i);
  }
}
