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
  void CachedSliceRendererTask::setInput(CachedSliceRenderer::CacheNode* node, RepresentationSList representations)
  {
    m_node = node;
    m_position = node->position;

    struct CachedSliceRenderer::ActorData dummy;
    for(auto rep: representations)
      m_representations.insert(rep, dummy);
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::computeData(RepresentationSPtr representation)
  {
    if (ChannelSliceCachedRepresentation::TYPE == representation->type())
    {
      auto channelRep = std::dynamic_pointer_cast<ChannelSliceCachedRepresentation>(representation);
      if (m_representations[representation].actor == nullptr && m_representations[representation].time != channelRep->getModificationTime())
      {
        m_representations[representation].actor = channelRep->getActor(m_position);
        m_representations[representation].time  = channelRep->getModificationTime();;
      }
    }

    if (SegmentationSliceCachedRepresentation::TYPE == representation->type())
    {
      auto segRep = std::dynamic_pointer_cast<SegmentationSliceCachedRepresentation>(representation);
      if (m_representations[representation].actor == nullptr && m_representations[representation].time != segRep->getModificationTime())
      {
        m_representations[representation].actor = segRep->getActor(m_position);
        m_representations[representation].time = segRep->getModificationTime();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::releaseActors()
  {
    for(auto rep: m_representations.keys())
      m_representations[rep].actor = nullptr;

    m_representations.clear();
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRendererTask::run()
  {
    int count = 0;

    for (auto rep : m_representations.keys())
    {
      if (!canExecute())
        return;

      computeData(rep);

      ++count;
      emit progress((count * 100) / m_representations.size());
    }

    m_node->mutex.lock();
    if (m_node->position != m_position || m_node->worker == nullptr || m_node->worker.get() != this)
    {
      m_node->mutex.unlock();
      releaseActors();
      return;
    }

    for (auto rep: m_representations.keys())
      m_node->representations[rep] = m_representations[rep];

    m_node->mutex.unlock();
    emit ready(m_node);
  }

} /* namespace EspINA */
