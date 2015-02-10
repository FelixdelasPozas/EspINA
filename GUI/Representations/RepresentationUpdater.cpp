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
, m_pipeline      {pipeline}
, m_timeStamp     {0}
, m_timeStampValid{false}
{
  setHidden(true);

}

//----------------------------------------------------------------------------
void RepresentationUpdater::addSource(ViewItemAdapterPtr item)
{
  Q_ASSERT(!m_states.contains(item));

  m_states[item] = RepresentationPipeline::State();
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
bool RepresentationUpdater::applySettings(const RepresentationPipeline::State &settings)
{
  bool modified = false;

  for (auto &state : m_states)
  {
    state.apply(settings);
    modified |= state.hasPendingChanges(); // TODO when to do the commit?
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
void RepresentationUpdater::invalidateTimeStamp()
{
  m_timeStampValid = false;
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
  m_actors.clear();
  //qDebug() << "Task" << description() << "running" << " - " << this;
  auto it = m_states.begin();

  while (canExecute() && it != m_states.end())
  {
    auto tmpRep = it.key()->temporalRepresentation();

    if (tmpRep && tmpRep->type() == m_pipeline->type())
    {
      m_actors << tmpRep->createActors(it.key(), it.value());
    }
    else
    {
      m_actors << m_pipeline->createActors(it.key(), it.value());
    }

    ++it;
  }

  if (hasValidTimeStamp() && canExecute()) {
    emit actorsUpdated(timeStamp(), m_actors);
  }
}