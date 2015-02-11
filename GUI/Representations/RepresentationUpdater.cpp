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
#include <QCoreApplication>

using namespace ESPINA;

//----------------------------------------------------------------------------
RepresentationUpdater::RepresentationUpdater(SchedulerSPtr scheduler,
                                             RepresentationPipelineSPtr pipeline)
: Task            {scheduler}
, m_timeStamp     {0}
, m_timeStampValid{false}
, m_pipeline      {pipeline}
{
  setHidden(true);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::addSource(ViewItemAdapterPtr item)
{
  Q_ASSERT(!m_states.contains(item));

  m_states[item] = RepresentationState();
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removeSource(ViewItemAdapterPtr item)
{
  Q_ASSERT(m_states.contains(item));

  m_states.remove(item);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setCrosshair(const NmVector3 &point)
{
  for (auto &state : m_states)
  {
    setCrosshairPoint(point, state);
  }
}

//----------------------------------------------------------------------------
bool RepresentationUpdater::applySettings(const RepresentationState &settings)
{
  bool modified = false;

  auto it = m_states.begin();

  while (it != m_states.end())
  {
    auto  item     = it.key();
    auto &state    = it.value();
    auto  pipeline = sourcePipeline(item);

    modified |= pipeline->applySettings(item, settings, state);

    ++it;
  }

  return modified;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setTimeStamp(TimeStamp time)
{
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

//   for (auto &state : m_states)
//   {
//     state.clear();
//   }
}

//----------------------------------------------------------------------------
bool RepresentationUpdater::hasValidTimeStamp() const
{
  return m_timeStampValid;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList RepresentationUpdater::actors() const
{
  return m_actors;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::run()
{
  qDebug() << "Task" << description() << "running" << " - " << this;

  m_actors.clear();

  auto it = m_states.begin();

  while (canExecute() && it != m_states.end())
  {
    auto  item     = it.key();
    auto &state    = it.value();
    auto  pipeline = sourcePipeline(item);

    state.commit();

    m_actors << pipeline->createActors(item, state);

    ++it;
  }

  qDebug() << "Task:" << description() << "has" << m_actors.size() << "actors" << " - needsrestart" << needsRestart();
  if (hasValidTimeStamp() && canExecute())
  {
    emit actorsUpdated(timeStamp(), m_actors);
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
