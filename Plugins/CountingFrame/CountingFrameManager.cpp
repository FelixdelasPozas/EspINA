/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "CountingFrameManager.h"
#include "Extensions/CountingFrameExtension.h"
#include "CountingFrames/AdaptiveCountingFrame.h"
#include "CountingFrames/OrthogonalCountingFrame.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Category.h>
#include <Core/Utils/AnalysisUtils.h>

// Qt
#include <QMutex>

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFrameExtensionSPtr CountingFrameManager::createExtension(SchedulerSPtr scheduler,
                                                                 const State& state) const
{
  return std::make_shared<CountingFrameExtension>(const_cast<CountingFrameManager *>(this), scheduler, state);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf)
{
  m_countingFrames[cf] = cf->extension()->extendedItem();

  emit countingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::unregisterCountingFrame(CountingFrame* cf)
{
  m_countingFrames.remove(cf);

  emit countingFrameDeleted(cf);
}


//-----------------------------------------------------------------------------
CountingFrame::Id CountingFrameManager::defaultCountingFrameId(const QString &constraint) const
{
  CountingFrame::Id id = constraint;

  if (id.isEmpty())
  {
    id = tr("Global");
  }

  return SuggestId(id, m_countingFrames.keys());
}

//-----------------------------------------------------------------------------
void CountingFrameManager::clearEdges()
{
  m_edges.clear();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> CountingFrameManager::edges(ChannelPtr channel)
{
  QMutexLocker lock(&m_edgesMutex);

  if (!m_edges.keys().contains(channel))
  {
    // TODO: rework this so this extension has access to ChannelEdges extension information on load
    // right now it's a deadlock.
    ChannelEdges edgesExtension;
    edgesExtension.setExtendedItem(channel);

    m_edges.insert(channel, edgesExtension.channelEdges());
  }

  return m_edges[channel];
}
