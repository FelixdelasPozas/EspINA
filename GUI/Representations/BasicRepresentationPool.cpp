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

//-----------------------------------------------------------------------------
template<typename P, typename S>
ESPINA::BasicRepresentationPool<P, S>::BasicRepresentationPool(S settings, SchedulerSPtr scheduler)
: m_settings{settings}
, m_updater{std::make_shared<RepresentationUpdater>(scheduler)}
{
  connect(m_updater.get(), SIGNAL(finished()),
          this,            SIGNAL(representationsReady()));
}

//-----------------------------------------------------------------------------
template<typename P, typename S>
void ESPINA::BasicRepresentationPool<P, S>::setCrosshair(const NmVector3 &point)
{
  m_updater->setCroshair(point);
}

//-----------------------------------------------------------------------------
template<typename P, typename S>
bool ESPINA::BasicRepresentationPool<P, S>::isReady() const
{
  return m_updater->hasFinished() && !m_updater->needsRestart();
}

//-----------------------------------------------------------------------------
template<typename P, typename S>
ESPINA::RepresentationPipelineSList ESPINA::BasicRepresentationPool<P, S>::pipelines()
{
  return m_updater->pipelines();
}

//-----------------------------------------------------------------------------
template<typename P, typename S>
void ESPINA::BasicRepresentationPool<P, S>::addRepresentationPipeline(ViewItemAdapterPtr source)
{
  m_updater->addPipeline(source, std::make_shared<P>(source));
}

//-----------------------------------------------------------------------------
template<typename P, typename S>
void ESPINA::BasicRepresentationPool<P, S>::updateImplementation()
{
  m_updater->applySettings(m_settings->settings());

  Task::submit(m_updater);
}

