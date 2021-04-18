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

#include "ChunkReporter.h"
#include <Support/Widgets/ProgressTool.h>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
ChunkReporter::ChunkReporter(unsigned int chunks, ProgressTool* tool)
: m_completedChunks{0}
, m_chunkProgress  {100.0f/chunks}
, m_tool           {tool}
{}

//----------------------------------------------------------------------------
void ChunkReporter::nextChunk()
{
  ++m_completedChunks;
  setProgress(0);
}

//----------------------------------------------------------------------------
void ChunkReporter::setProgress(unsigned int value)
{
  auto totalValue = (m_completedChunks + value/100.0)*m_chunkProgress;
  m_tool->setProgress(totalValue);

  emit progress(totalValue);
}
