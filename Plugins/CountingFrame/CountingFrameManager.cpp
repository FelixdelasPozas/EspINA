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
#include <GUI/View/ViewState.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// Qt
#include <QMutex>

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFrameManager::CountingFrameManager(Support::Context& context)
: m_context(context)
{
}

//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf)
{
  m_countingFrames[cf] = cf->extension()->extendedItem();

  connect(cf, SIGNAL(applied(CountingFrame *)), this, SLOT(onCountingFrameApplied(CountingFrame *)));

  emit countingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::unregisterCountingFrame(CountingFrame* cf)
{
  m_countingFrames.remove(cf);

  disconnect(cf, SIGNAL(applied(CountingFrame *)), this, SLOT(onCountingFrameApplied(CountingFrame *)));

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
void CountingFrameManager::onCountingFrameApplied(CountingFrame *cf)
{
  auto segmentations = m_context.model()->segmentations();

  ViewItemAdapterList updated;
  auto constraint = cf->categoryConstraint();

  for(auto segmentation: segmentations)
  {
    if(constraint.isEmpty() || (segmentation->category()->classificationName().startsWith(constraint)))
    {
      updated << segmentation.get();
    }
  }

  m_context.viewState().invalidateRepresentationColors(updated);
}
