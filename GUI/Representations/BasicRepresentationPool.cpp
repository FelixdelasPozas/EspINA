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
  template<typename P>
  BasicRepresentationPool<P>::BasicRepresentationPool(SchedulerSPtr scheduler)
  : m_updater      {std::make_shared<RepresentationUpdater>(scheduler, std::make_shared<P>())}
  , m_init         {false}
  , m_hasChanged   {false}
  {
    connect(m_updater.get(), SIGNAL(actorsReady(TimeStamp,RepresentationPipeline::Actors)),
            this,            SLOT(onActorsReady(TimeStamp,RepresentationPipeline::Actors)), Qt::DirectConnection);

    m_updater->setDescription(QString("Channel 3D Lines"));
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::setResolution(const NmVector3 &resolution)
  {
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  ViewItemAdapterPtr BasicRepresentationPool<P>::pick(const NmVector3 &point, vtkProp *actor) const
  {
    return m_updater->pick(point, actor);
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::addRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->addSource(source);
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::removeRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->removeSource(source);
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::setCrosshairImplementation(const NmVector3 &point, TimeStamp t)
  {
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
  template<typename P>
  void BasicRepresentationPool<P>::onSettingsChanged(const RepresentationState &settings)
  {
    m_updater->setSettings(settings);

    Task::submit(m_updater);
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  bool BasicRepresentationPool<P>::actorsChanged() const
  {
    return m_hasChanged;
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::invalidateImplementation()
  {
    if (m_init)
    {
      m_hasChanged = true;

      Task::submit(m_updater);
    }
  }

  //-----------------------------------------------------------------------------
  template<typename P>
  void BasicRepresentationPool<P>::invalidateRepresentations(ViewItemAdapterList items, TimeStamp t)
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
