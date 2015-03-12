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

using namespace ESPINA;

//-----------------------------------------------------------------------------
BufferedRepresentationPool::BufferedRepresentationPool(const Plane                plane,
                                                       RepresentationPipelineSPtr pipeline,
                                                       SchedulerSPtr              scheduler,
                                                       unsigned                   windowSize)
: m_normalIdx{normalCoordinateIndex(plane)}
, m_updateWindow{scheduler, pipeline, windowSize}
, m_init{false}
, m_normalRes{1}
, m_hasChanged{false}
, m_changedTimeStamp{0}
{
  connect(&m_updateWindow, SIGNAL(actorsReady(TimeStamp,RepresentationPipeline::Actors)),
          this,            SLOT(onActorsReady(TimeStamp,RepresentationPipeline::Actors)), Qt::DirectConnection);
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::setResolution(const NmVector3 &resolution)
{
  auto normalRes = resolution[m_normalIdx];

  if (m_normalRes != normalRes)
  {
    m_normalRes = normalRes;

    if (m_init)
    {
      m_hasChanged = true;

      auto invalidated = updateBuffer(m_crosshair, invalidationShift(), m_changedTimeStamp);

      updatePipelines(invalidated);
    }
  }
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::addRepresentationPipeline(ViewItemAdapterPtr source)
{
  for (auto updater : m_updateWindow.all())
  {
    updater->addSource(source);
  }
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::removeRepresentationPipeline(ViewItemAdapterPtr source)
{
  for (auto updater : m_updateWindow.all())
  {
    updater->removeSource(source);
  }
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::setCrosshairImplementation(const NmVector3 &point, TimeStamp t)
{
  auto shift = m_init?distanceFromLastCrosshair(point):invalidationShift();

  m_init     = true;

  if (t > m_changedTimeStamp)
  {
    m_hasChanged       = shift != 0;
    m_changedTimeStamp = t;
  }

  auto invalidated = updateBuffer(point, shift, t);

  updatePipelines(invalidated);
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::onSettingsChanged(const RepresentationState &settings)
{
  auto invalidated =  m_updateWindow.all();

  for (auto updater : invalidated)
  {
    updater->setSettings(settings);
  }

  updatePipelines(invalidated);
}

//-----------------------------------------------------------------------------
bool BufferedRepresentationPool::actorsChanged() const
{
  return m_hasChanged;
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::invalidateImplementation()
{
  if (m_init)
  {
    m_hasChanged = true;

    updatePipelines(m_updateWindow.all());
  }
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::invalidateRepresentations(ViewItemAdapterList items, TimeStamp t)
{
  if(m_init)
  {
    m_hasChanged = true;

    m_updateWindow.current()->setTimeStamp(t);

    auto updaters = m_updateWindow.all();

    for(auto updater: updaters)
    {
      updater->setUpdateList(items);
    }

    updatePipelines(updaters);
  }
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::updatePriorities()
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
int BufferedRepresentationPool::distanceFromLastCrosshair(const NmVector3 &crosshair)
{
  return vtkMath::Round((normal(crosshair) - normal(m_crosshair))/m_normalRes);
}

//-----------------------------------------------------------------------------
Nm BufferedRepresentationPool::normal(const NmVector3 &point) const
{
  return point[m_normalIdx];
}

//-----------------------------------------------------------------------------
NmVector3 BufferedRepresentationPool::representationCrosshair(const NmVector3 &point, int shift) const
{
  NmVector3 crosshair = point;

  crosshair[m_normalIdx] += shift*m_normalRes;

  return crosshair;
}

//-----------------------------------------------------------------------------
RepresentationUpdaterSList BufferedRepresentationPool::updateBuffer(const NmVector3 &point,
                                                                    int   shift,
                                                                    const TimeStamp t)
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
    //qDebug() << this << "Invalidating:" << old << " to " << updateTask->description() ;

    invalidated << updateTask;
  }

  m_updateWindow.current()->setTimeStamp(t);
  m_crosshair = point;

  return invalidated;
}

//-----------------------------------------------------------------------------
ViewItemAdapterPtr BufferedRepresentationPool::pick(const NmVector3 &point,
                                                    vtkProp *actor) const
{
  ViewItemAdapterPtr pickedItem = nullptr;

  if (m_updateWindow.current()->hasFinished())
  {
    pickedItem = m_updateWindow.current()->pick(point, actor);
  }

  return pickedItem;
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::updatePipelines(RepresentationUpdaterSList updaters)
{
  updatePriorities();

  for (auto updater : updaters)
  {
    Task::submit(updater);
  }

  checkCurrentActors();
}

//-----------------------------------------------------------------------------
void BufferedRepresentationPool::checkCurrentActors()
{
  auto current = m_updateWindow.current();

  if (current->hasFinished())
  {
    onActorsReady(current->timeStamp(), current->actors());
  }
}

//-----------------------------------------------------------------------------
int BufferedRepresentationPool::invalidationShift() const
{
  return m_updateWindow.size() + 1;
}