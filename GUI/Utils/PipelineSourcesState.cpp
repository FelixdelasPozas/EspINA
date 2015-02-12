/*
 Copyright (C) 2015  Felix de las Pozas Alvarez

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "PipelineSourcesState.h"
#include <GUI/Representations/PipelineSources.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  PipelineSourcesState::PipelineSourcesState(TimerSPtr timer)
  : QObject()
  , m_timer{timer}
  {
  }

  //----------------------------------------------------------------------------
  void PipelineSourcesState::addSource(PipelineSources *source)
  {
    if(m_sources.contains(source)) return;

    connect(source, SIGNAL(sourceAdded(ViewItemAdapterPtr)),
            this,   SLOT(onSourceAdded(ViewItemAdapterPtr)));
    connect(source, SIGNAL(sourceUpdated(ViewItemAdapterPtr)),
            this,   SLOT(onSourceUpdated(ViewItemAdapterPtr)));
    connect(source, SIGNAL(sourceRemoved(ViewItemAdapterPtr)),
            this,   SLOT(onSourceRemoved(ViewItemAdapterPtr)));

    connect(this,   SIGNAL(sourceAdded(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
            source, SLOT(onSourceAdded(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));
    connect(this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
            source, SLOT(onSourceUpdated(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));
    connect(this,   SIGNAL(sourceRemoved(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
            source, SLOT(onSourceRemoved(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));

    m_sources << source;
  }

  //----------------------------------------------------------------------------
  void PipelineSourcesState::removeSource(PipelineSources *source)
  {
    if(!m_sources.contains(source)) return;

    disconnect(source, SIGNAL(sourceAdded(ViewItemAdapterPtr)),
               this,   SLOT(onSourceAdded(ViewItemAdapterPtr)));
    disconnect(source, SIGNAL(sourceUpdated(ViewItemAdapterPtr)),
               this,   SLOT(onSourceUpdated(ViewItemAdapterPtr)));
    disconnect(source, SIGNAL(sourceRemoved(ViewItemAdapterPtr)),
               this,   SLOT(onSourceRemoved(ViewItemAdapterPtr)));

    disconnect(this,   SIGNAL(sourceAdded(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
               source, SLOT(onSourceAdded(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));
    disconnect(this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
               source, SLOT(onSourceUpdated(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));
    disconnect(this,   SIGNAL(sourceRemoved(ViewItemAdapterPtr, PipelineSources*, TimeStamp)),
               source, SLOT(onSourceRemoved(ViewItemAdapterPtr, PipelineSources*, TimeStamp)));

    m_sources.removeOne(source);
  }

  //----------------------------------------------------------------------------
  void PipelineSourcesState::onSourceAdded(ViewItemAdapterPtr item) const
  {
    auto source = dynamic_cast<PipelineSources *>(sender());
    emit sourceAdded(item, source, m_timer->increment());
  }

  //----------------------------------------------------------------------------
  void PipelineSourcesState::onSourceUpdated(ViewItemAdapterPtr item) const
  {
    auto source = dynamic_cast<PipelineSources *>(sender());
    emit sourceUpdated(item, source, m_timer->increment());
  }

  //----------------------------------------------------------------------------
  void PipelineSourcesState::onSourceRemoved(ViewItemAdapterPtr item) const
  {
    auto source = dynamic_cast<PipelineSources *>(sender());
    emit sourceRemoved(item, source, m_timer->increment());
  }

} /* namespace ESPINA */
