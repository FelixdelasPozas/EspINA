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
: Task        {scheduler}
, m_frame     {Frame::InvalidFrame()}
, m_pipeline  {pipeline}
, m_updateList{&m_sources}
, m_actors    {std::make_shared<RepresentationPipeline::ActorsData>()}
{
  setHidden(true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::addSource(ViewItemAdapterPtr item)
{
  QWriteLocker lock(&m_dataLock);

  addUpdateRequest(m_sources, item, true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removeSource(ViewItemAdapterPtr item)
{
  {
    RepresentationPipeline::ActorsLocker actors(m_actors);

    if(actors.get().keys().contains(item))
    {
      actors.get().remove(item);
    }
  }

  QWriteLocker lock(&m_dataLock);
  removeUpdateRequest(m_sources, item);
  removeUpdateRequest(m_requestedSources, item);
}

//----------------------------------------------------------------------------
bool RepresentationUpdater::isEmpty() const
{
  QReadLocker lock(&m_dataLock);

  return m_sources.isEmpty();
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setCrosshair(const NmVector3 &point)
{
  QWriteLocker lock(&m_dataLock);

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
  QWriteLocker lock(&m_dataLock);

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
  QWriteLocker lock(&m_dataLock);

  m_frame = frame;
}

//----------------------------------------------------------------------------
FrameCSPtr RepresentationUpdater::frame() const
{
  QReadLocker lock(&m_dataLock);

  return m_frame;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::invalidate()
{
  QWriteLocker lock(&m_dataLock);

  m_frame = Frame::InvalidFrame();
}

//----------------------------------------------------------------------------
RepresentationPipeline::Actors RepresentationUpdater::actors() const
{
  return m_actors;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::run()
{
  FrameSPtr frame;
  RepresentationState settings;
  UpdateRequestList updateList;

  {
    QReadLocker lock(&m_dataLock);
    frame = m_frame;
    settings = m_settings;
  }

  {
    QWriteLocker lock(&m_dataLock);

    // Local copy needed to prevent condition race on same frame
    // (usually due to invalidation view item representations)
    updateList = *m_updateList;
    updateList.detach();

    m_updateList    = &m_sources;
    m_requestedSources.clear();
  }

  auto it   = updateList.begin();
  auto size = updateList.size();

  {
    RepresentationPipeline::ActorsLocker actors(m_actors);

    int i = 0;
    while (canExecute() && it != updateList.end())
    {
      auto item     = it->first;
      auto pipeline = sourcePipeline(item);

      auto state = pipeline->representationState(item, m_settings);

      if (it->second)
      {
        actors.get()[item] = pipeline->createActors(item, state);
      }

      pipeline->updateColors(actors.get()[item], item, state);

      ++it;
      ++i;

      reportProgress((i/static_cast<double>(size))*100);
    }
  }

  if (isValid(frame) && canExecute())
  {
    emit actorsReady(frame, m_actors);
  }
}

//----------------------------------------------------------------------------
RepresentationPipelineSPtr RepresentationUpdater::sourcePipeline(ViewItemAdapterPtr item) const
{
  QWriteLocker lock(&m_dataLock);

  auto pipeline = item->temporalRepresentation();

  if (!pipeline || pipeline->type() != m_pipeline->type())
  {
    pipeline = m_pipeline;
  }

  return pipeline;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::updateRepresentations(ViewItemAdapterList sources, const bool createActors)
{
  QWriteLocker lock(&m_dataLock);

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
