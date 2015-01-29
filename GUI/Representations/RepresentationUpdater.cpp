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

using namespace ESPINA;

//----------------------------------------------------------------------------
RepresentationUpdater::RepresentationUpdater(SchedulerSPtr scheduler)
: Task(scheduler)
{

}

//----------------------------------------------------------------------------
void RepresentationUpdater::addPipeline(ViewItemAdapterPtr item, RepresentationPipelineSPtr pipeline)
{
  Q_ASSERT(!m_multiplexers.contains(item));

  m_multiplexers[item] = std::make_shared<PipelineMultiplexer>(item, pipeline);
}

//----------------------------------------------------------------------------
void RepresentationUpdater::removePipeline(ViewItemAdapterPtr item)
{
}

//----------------------------------------------------------------------------
void RepresentationUpdater::setCroshair(const NmVector3 &point)
{
  for (auto pipeline : pipelines())
  {
    pipeline->setCrosshairPoint(point);
  }
}

//----------------------------------------------------------------------------
void RepresentationUpdater::applySettings(const RepresentationPipeline::Settings &state)
{
  for (auto pipeline : pipelines())
  {
    pipeline->applySettings(state);
  }
}

//----------------------------------------------------------------------------
RepresentationPipelineSList RepresentationUpdater::pipelines()
{
  RepresentationPipelineSList pipelines;

  for (auto multiplexer : m_multiplexers)
  {
    pipelines << multiplexer->active();
  }

  return pipelines;
}

//----------------------------------------------------------------------------
void RepresentationUpdater::run()
{
  for (auto pipeline : pipelines())
  {
    if (!canExecute()) break;

    pipeline->update();
  }
}
