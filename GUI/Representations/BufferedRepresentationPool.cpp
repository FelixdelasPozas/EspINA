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
ESPINA::BufferedRepresentationPool<P>::BufferedRepresentationPool(SchedulerSPtr scheduler, unsigned windowSize)
: m_updateWindow{scheduler, windowSize}
{
  connect(m_updateWindow.current().get(), SIGNAL(finished()),
          this,                           SIGNAL(representationsReady()));
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::setCrosshair(const NmVector3 &point)
{
  //auto shift = shiftFromPoint(point);
  int shift = 0;
  for (auto updater : m_updateWindow.all())
  {
    updater->setCroshair(point);
  }

//   for (auto cursor : m_updateWindow.moveCurrent(shift))
//   {
//     NmVector3 crosshair; // f(cursor.second)
//     cursor.first->setCrosshair(crosshair);
//   }
}

//-----------------------------------------------------------------------------
template<typename P>
ESPINA::RepresentationPipelineSList ESPINA::BufferedRepresentationPool<P>::pipelines()
{
  return m_updateWindow.current()->pipelines();
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::addRepresentationPipeline(ViewItemAdapterPtr source)
{
  for (auto updater : m_updateWindow.all())
  {
    updater->addPipeline(source, std::make_shared<P>(source));
  }
}

//-----------------------------------------------------------------------------
template<typename P>
bool ESPINA::BufferedRepresentationPool<P>::isReadyImplementation() const
{
  return m_updateWindow.current()->hasFinished();
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::updateImplementation()
{
  updateWindowPosition(m_updateWindow.current(), Priority::VERY_HIGH);

  for (auto closest : m_updateWindow.closest())
  {
    updateWindowPosition(closest, Priority::HIGH);
  }

  for (auto closest : m_updateWindow.further())
  {
    updateWindowPosition(closest, Priority::NORMAL);
  }
}

//-----------------------------------------------------------------------------
template<typename P>
void ESPINA::BufferedRepresentationPool<P>::updateWindowPosition(RepresentationUpdaterSPtr updater, Priority priority)
{
  if (updater->applySettings(settings()))
  {
    updater->setPriority(priority);

    Task::submit(updater);
  }
}

