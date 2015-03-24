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

#include "BasicRepresentationPool.h"

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  BasicRepresentationPool::BasicRepresentationPool(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline)
  : m_updater      {std::make_shared<RepresentationUpdater>(scheduler, pipeline)}
  , m_init         {false}
  , m_hasChanged   {false}
  , m_static       {false}
  {
    connect(m_updater.get(), SIGNAL(actorsReady(TimeStamp,RepresentationPipeline::Actors)),
            this,            SLOT(onActorsReady(TimeStamp,RepresentationPipeline::Actors)), Qt::DirectConnection);
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::setResolution(const NmVector3 &resolution)
  {
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterPtr BasicRepresentationPool::pick(const NmVector3 &point, vtkProp *actor) const
  {
    return m_updater->pick(point, actor);
  }

  //-----------------------------------------------------------------------------
  bool BasicRepresentationPool::isStatic() const
  {
    return m_static;
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::setStaticRepresentation(bool value)
  {
    m_static = value;
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::addRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->addSource(source);
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::removeRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->removeSource(source);
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::setCrosshairImplementation(const NmVector3 &point, TimeStamp t)
  {
    if(m_static && !m_init)
    {

      Task::submit(m_updater);
    }

    m_init = true;


    if (t > lastUpdateTimeStamp())
    {
      m_hasChanged = true;

      m_updater->invalidate();
      m_updater->setCrosshair(point);
      m_updater->setTimeStamp(t);

      Task::submit(m_updater);
    }
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::onSettingsChanged(const RepresentationState &settings)
  {
    m_updater->setSettings(settings);

    Task::submit(m_updater);
  }

  //-----------------------------------------------------------------------------
  bool BasicRepresentationPool::actorsChanged() const
  {
    return m_hasChanged;
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::invalidateImplementation()
  {
    if (m_init)
    {
      m_hasChanged = true;

      Task::submit(m_updater);
    }
  }

  //-----------------------------------------------------------------------------
  void BasicRepresentationPool::invalidateRepresentations(ViewItemAdapterList items, TimeStamp t)
  {
    if(m_init)
    {
      m_hasChanged = true;

      m_updater->setTimeStamp(t);
      m_updater->setUpdateList(items);

      Task::submit(m_updater);
    }
  }
}
