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
#include <QMutexLocker>

// C++
#include <algorithm>

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFrameManager::CountingFrameManager()
: m_context{nullptr}
, m_applyTask{nullptr}
{
}

//-----------------------------------------------------------------------------
void CountingFrameManager::registerCountingFrame(CountingFrame* cf)
{
  CFData data;
  data.channel   = cf->extension()->extendedItem();
  data.needApply = false;

  m_countingFrames.insert(cf, data);

  connect(cf, SIGNAL(apply(CountingFrame *)), this, SLOT(applyCountingFrame(CountingFrame *)));

  emit countingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::unregisterCountingFrame(CountingFrame* cf)
{
  m_countingFrames.remove(cf);

  {
    QMutexLocker lock(&m_taskMutex);
    if(m_applyTask && m_applyTask->countingFrame() == cf)
    {
      m_applyTask->abort();
    }
  }

  disconnect(cf, SIGNAL(apply(CountingFrame *)), this, SLOT(applyCountingFrame(CountingFrame *)));

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

  return suggestedId(id);
}

//-----------------------------------------------------------------------------
void CountingFrameManager::onCountingFrameApplied()
{
  auto task = qobject_cast<ApplyCountingFrame *>(sender());
  if(task)
  {
    auto cf = task->countingFrame();
    const auto hasBeenAborted = task->isAborted();

    {
      QMutexLocker lock(&m_taskMutex);
      m_applyTask = nullptr;
    }

    if(cf && m_countingFrames.keys().contains(cf))
    {
      m_countingFrames[cf].needApply = false;
    }

    if(cf && m_context && !hasBeenAborted)
    {
      auto segmentations = m_context->model()->segmentations();

      ViewItemAdapterList updated;
      auto constraint = cf->categoryConstraint();

      for(auto segmentation: segmentations)
      {
        if(constraint.isEmpty() || (segmentation->category()->classificationName().startsWith(constraint)))
        {
          updated << segmentation.get();
        }
      }

      m_context->viewState().invalidateRepresentationColors(updated);
      cf->applied();
    }

    checkApplies();
  }
}

//-----------------------------------------------------------------------------
CountingFrame::Id CountingFrameManager::suggestedId(const CountingFrame::Id &id) const
{
  return SuggestId(id, m_countingFrames.keys());
}

//-----------------------------------------------------------------------------
CountingFrameList CountingFrameManager::countingFrames() const
{
  auto cFrames = m_countingFrames.keys();
  std::sort(cFrames.begin(), cFrames.end(), CF::lessThan);

  return cFrames;
}

//-----------------------------------------------------------------------------
void CountingFrameManager::applyCountingFrame(CountingFrame *cf)
{
  m_countingFrames[cf].needApply = true;

  checkApplies();
}

//-----------------------------------------------------------------------------
void CountingFrameManager::checkApplies()
{
  QMutexLocker lock(&m_taskMutex);

  if(!m_applyTask)
  {
    auto needsApply = [](const CFData &d) { return d.needApply; };
    auto it = std::find_if(m_countingFrames.cbegin(), m_countingFrames.cend(), needsApply);

    if(it != m_countingFrames.cend())
    {
      m_applyTask = std::make_shared<ApplyCountingFrame>(it.key(), m_factory, m_context->scheduler());

      connect(m_applyTask.get(), SIGNAL(finished()),
              this,              SLOT(onCountingFrameApplied()));

      Task::submit(m_applyTask);
    }
  }
}
