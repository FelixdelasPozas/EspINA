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

#include "BufferedRepresentationPool.h"


//-----------------------------------------------------------------------------
template<typename P>
ESPINA::BufferedRepresentationPool<P>::BufferedRepresentationPool(const Plane   plane,
                                                                  SchedulerSPtr scheduler,
                                                                  unsigned      windowSize)
: m_normalIdx{normalCoordinateIndex(plane)}
, m_updateWindow{scheduler, std::make_shared<P>(), windowSize}
, m_init{false}
, m_shift{0}
, m_normalRes{1}
, m_lastCoordinate{0}
{
  connect(&m_updateWindow, SIGNAL(actorsReady(TimeStamp,RepresentationPipeline::Actors)),
          this,            SLOT(onActorsReady(TimeStamp,RepresentationPipeline::Actors)), Qt::DirectConnection);
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::setResolution(const NmVector3 &resolution)
{
  m_normalRes = resolution[m_normalIdx];

  if (m_init)
  {
    //TODO
  }
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::addRepresentationPipeline(ViewItemAdapterPtr source)
{
  for (auto updater : m_updateWindow.all())
  {
    updater->addSource(source);
  }
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::setCrosshairImplementation(const NmVector3 &point, TimeStamp time)
{
  m_shift = m_init?distanceFromLastCrosshair(point):invalidationShift();

  m_init  = true;

  auto invalidated = updateBuffer(point, m_shift, time);

  updatePipelines(invalidated);
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::onSettingsChanged(const RepresentationState &settings)
{
  auto invalidated =  m_updateWindow.all();

  for (auto updater : invalidated)
  {
    updater->setSettings(settings);
  }

  updatePipelines(invalidated);
}

//-----------------------------------------------------------------------------
template<typename P>
bool ESPINA::BufferedRepresentationPool<P>::changed() const
{
  return m_shift != 0;
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::updatePriorities()
{
  m_updateWindow.current()->setPriority(Priority::VERY_HIGH);

  for (auto closest : m_updateWindow.closest())
  {
    closest->setPriority(Priority::HIGH);
  }

  for (auto further : m_updateWindow.further())
  {
    further->setPriority(Priority::LOW);
  }
}

//-----------------------------------------------------------------------------
template<typename P>
int ESPINA::BufferedRepresentationPool<P>::distanceFromLastCrosshair(const NmVector3 &crosshair)
{
  return vtkMath::Round((normal(crosshair) - m_lastCoordinate)/m_normalRes);
}

//-----------------------------------------------------------------------------
template<typename P>
int ESPINA::BufferedRepresentationPool<P>::normal(const NmVector3 &point) const
{
  return point[m_normalIdx];
}

//-----------------------------------------------------------------------------
template<typename P>
ESPINA::NmVector3 ESPINA::BufferedRepresentationPool<P>::representationCrosshair(const NmVector3 &point, int shift) const
{
  NmVector3 crosshair = point;

  crosshair[m_normalIdx] += shift*m_normalRes;

  return crosshair;
}

//-----------------------------------------------------------------------------
template<typename P>
ESPINA::RepresentationUpdaterSList ESPINA::BufferedRepresentationPool<P>::updateBuffer(const NmVector3 &point,
                                                                                       int   shift,
                                                                                       const TimeStamp time)
{
  RepresentationUpdaterSList invalidated;

  for (auto cursor : m_updateWindow.moveCurrent(shift))
  {
    auto updateTask = cursor.first;

    auto crosshair = representationCrosshair(point, cursor.second);
    auto old       = updateTask->description();

    updateTask->invalidate();
    updateTask->setCrosshair(crosshair);
    updateTask->setDescription(QString("Slice %1").arg(normal(crosshair)));
    qDebug() << "Invalidating:" << old << " to " << updateTask->description() ;

    invalidated << updateTask;
  }

  m_updateWindow.current()->setTimeStamp(time);
  m_lastCoordinate = normal(point);

  return invalidated;
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::updatePipelines(RepresentationUpdaterSList updaters)
{
  updatePriorities();

  for (auto updater : updaters)
  {
    Task::submit(updater);
  }

  checkCurrentActors();
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::checkCurrentActors()
{
  auto current = m_updateWindow.current();

  if (current->hasFinished())
  {
    onActorsReady(current->timeStamp(), current->actors());
  }
}

//-----------------------------------------------------------------------------
template<typename P>
int ESPINA::BufferedRepresentationPool<P>::invalidationShift() const
{
  return m_updateWindow.size() + 1;
}