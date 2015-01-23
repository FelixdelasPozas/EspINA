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

#include "PipelineMultiplexer.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
ESPINA::PipelineMultiplexer::PipelineMultiplexer(ViewItemAdapterPtr item, RepresentationPipelineSPtr pipeline)
: m_tmpPipelineActive{false}
, m_pipeline{pipeline}
{
  Q_ASSERT(m_pipeline);

  connect(item, SIGNAL(activateTemporalPipeline(RepresentationPipelineSPtr)),
          this, SLOT(onTemporalPipelineActivated(RepresentationSPtr)));
}

//-----------------------------------------------------------------------------
RepresentationPipelineSPtr PipelineMultiplexer::active() const
{
  return m_tmpPipelineActive?m_tmpPipeline:m_pipeline;
}

//-----------------------------------------------------------------------------
void PipelineMultiplexer::onTemporalPipelineActivated(RepresentationSPtr pipeline)
{

}

//-----------------------------------------------------------------------------
void PipelineMultiplexer::onTemporalPipelineDeactivated(RepresentationSPtr pipeline)
{

}

//-----------------------------------------------------------------------------
