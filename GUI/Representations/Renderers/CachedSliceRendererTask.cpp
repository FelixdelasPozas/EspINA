/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// EspINA
#include "CachedSliceRendererTask.h"
#include <GUI/Representations/SliceCachedRepresentation.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  CachedSliceRendererTask::CachedSliceRendererTask(SchedulerSPtr scheduler)
  : Task{scheduler}
  , m_executionTime{0}
  , m_node{nullptr}
  , m_position{0}
  {
  }

  //-----------------------------------------------------------------------------
  CachedSliceRendererTask::~CachedSliceRendererTask()
  {
    releaseActors();
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::setInput(CachedSliceRenderer::CacheNode* node, CachedRepresentationSList representations)
  {
    m_node = node;
    m_position = node->position;

    for(auto rep: representations)
      m_representations[rep] = nullptr;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::computeData(CachedRepresentationSPtr representation)
  {
    if (ChannelSliceCachedRepresentation::TYPE == representation->type())
    {
      auto channelRep = std::dynamic_pointer_cast<ChannelSliceCachedRepresentation>(representation);
      m_representations[representation] = channelRep->getActor(m_position);
    }

    if (SegmentationSliceCachedRepresentation::TYPE == representation->type())
    {
      auto segRep = std::dynamic_pointer_cast<SegmentationSliceCachedRepresentation>(representation);
      m_representations[representation] = segRep->getActor(m_position);
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::releaseActors()
  {
    for(auto rep: m_representations.keys())
      m_representations[rep] = nullptr;

    m_representations.clear();
  }

  //-----------------------------------------------------------------------------
  bool CachedSliceRendererTask::needToRestart()
  {
    bool result = m_node->restart;

    if (result)
    {
      m_node->mutex.lockForWrite();
      m_position = m_node->position;
      m_node->restart = false;
      m_node->mutex.unlock();
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::run()
  {
    int count = 0;

    bool pendingData = true;

    while(pendingData)
    {
      pendingData = false;
      for (auto rep : m_representations.keys())
      {
        if (!canExecute())
          return;

        if (needToRestart())
        {
          pendingData = true;
          break;
        }

        computeData(rep);

        ++count;
        emit progress((count * 100) / m_representations.size());
      }
    }

    if (!canExecute())
      return;

    m_node->mutex.lockForWrite();
    if (m_node->position != m_position || m_node->worker == nullptr || m_node->worker.get() != this || m_node->restart)
      releaseActors();
    else
    {
      for (auto rep: m_representations.keys())
        m_node->representations[rep] = m_representations[rep];
    }
    m_node->mutex.unlock();

    emit ready(m_node);
  }

} /* namespace EspINA */
