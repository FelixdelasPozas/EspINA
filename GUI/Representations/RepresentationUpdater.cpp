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
#include "Frame.h"
#include <vtkProp.h>
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

  if(m_actors.keys().contains(item))
  {
    m_actors.remove(item);
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

  qDebug() << "Task" << description() << "running" << " - " << crosshairPoint(m_settings);

  // Local copy needed to prevent condition race on same TimeStamp
  // (usually due to invalidation view item representations)

  auto updateList = *m_updateList;
  m_updateList    = &m_sources;

  auto it   = updateList.begin();
  auto size = updateList.size();

  int i  = 0;

  while (canExecute() && it != updateList.end())
  {
    auto item     = it->first;
    auto pipeline = sourcePipeline(item);

    auto state = pipeline->representationState(item, m_settings);

    if (it->second)
    {
      auto actors = pipeline->createActors(item, state);

      m_actors[item]  = actors;
    }

    pipeline->updateColors(m_actors[item], item, state);

    ++it;
    ++i;

    reportProgress((i/static_cast<double>(size))*100);
  }

  if (m_frame->isValid() && canExecute())
  {
    m_requestedSources.clear();

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

//----------------------------------------------------------------------------
void RepresentationUpdater::updateRepresentations(ViewItemAdapterList sources, const bool createActors)
{
  QMutexLocker lock(&m_mutex);

  for(auto item: sources)
  {
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