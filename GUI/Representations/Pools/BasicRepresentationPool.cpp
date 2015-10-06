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
#include <GUI/Representations/Pools/BasicRepresentationPool.h>
#include <GUI/Representations/Frame.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
BasicRepresentationPool::BasicRepresentationPool(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline)
: m_updater{std::make_shared<RepresentationUpdater>(scheduler, pipeline)}
{
  connect(m_updater.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)),
          this,            SLOT(onActorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)), Qt::DirectConnection);
}

//-----------------------------------------------------------------------------
ViewItemAdapterList BasicRepresentationPool::pick(const NmVector3 &point, vtkProp *actor) const
{
  return m_updater->pick(point, actor);
}

//-----------------------------------------------------------------------------
void BasicRepresentationPool::updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame)
{
  m_updater->invalidate();
  m_updater->setCrosshair(frame->crosshair);
  m_updater->setResolution(frame->resolution);
  m_updater->setFrame(frame);

  updateRepresentations();
}

//-----------------------------------------------------------------------------
void BasicRepresentationPool::updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
{
  m_updater->setFrame(frame);
  m_updater->updateRepresentations(modifiedItems);

  updateRepresentations();
}

//-----------------------------------------------------------------------------
void BasicRepresentationPool::updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
{
  m_updater->setFrame(frame);
  m_updater->updateRepresentationColors(modifiedItems);

  updateRepresentations();
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
void BasicRepresentationPool::applySettings(const RepresentationState &settings)
{
  m_updater->setSettings(settings);

  updateRepresentations();
}

//-----------------------------------------------------------------------------
void BasicRepresentationPool::updateRepresentations()
{
  if(isEnabled())
  {
    emit taskStarted(m_updater);
    Task::submit(m_updater);
  }
}
