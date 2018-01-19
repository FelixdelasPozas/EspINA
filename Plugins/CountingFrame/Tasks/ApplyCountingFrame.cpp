/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// Plugin
#include "ApplyCountingFrame.h"

// ESPINA
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Category.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/MultiTasking/Scheduler.h>
#include <CountingFrames/CountingFrame.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/StereologicalInclusion.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/ModelFactory.h>

// Qt
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
ApplyCountingFrame::ApplyCountingFrame(CountingFrame *countingFrame,
                                       CoreFactory   *factory,
                                       SchedulerSPtr  scheduler)
: Task           {scheduler}
, m_countingFrame{countingFrame}
, m_factory      {factory}
{
  setDescription(tr("Applying CF: %1").arg(m_countingFrame->id()));
}

//------------------------------------------------------------------------
ApplyCountingFrame::~ApplyCountingFrame()
{
  abortTasks();
}

//------------------------------------------------------------------------
void ApplyCountingFrame::run()
{
  bool validExecution = false;

  {
    auto stack       = m_countingFrame->channel();
    auto segmentations = m_countingFrame->channel()->analysis()->segmentations();

    SegmentationSList validSegmentations;

    if (!segmentations.isEmpty())
    {
      auto constraint = m_countingFrame->categoryConstraint();

      for (auto segmentation: segmentations)
      {
        if (!canExecute()) break;

        bool validChannel = false;

        for(auto segStack: QueryContents::channels(segmentation))
        {
          validChannel |= (segStack.get() == stack);
        }

        if(validChannel && (constraint.isEmpty() || (segmentation->category()->classificationName().startsWith(constraint))))
        {
          validSegmentations << segmentation;
        }
      }
    }

    if(!validSegmentations.isEmpty())
    {
      auto maxTasks = Scheduler::maxRunningTasks();
      QVector<SegmentationSList> partitions(maxTasks);

      int i = 0;
      for(auto segmentation: validSegmentations)
      {
        if (!canExecute()) break;

        partitions[i++ % maxTasks] << segmentation;
      }

      for(unsigned int i = 0; i < maxTasks; ++i)
      {
        if (!canExecute()) break;

        struct Data data;
        data.Task     = std::make_shared<ApplySegmentationCountingFrame>(m_countingFrame, partitions[i], m_factory, m_scheduler);
        data.Progress = 0;

        connect(data.Task.get(), SIGNAL(progress(int, ApplySegmentationCountingFrame *)),
                this,            SLOT(onTaskProgress(int, ApplySegmentationCountingFrame *)), Qt::DirectConnection);

        connect(data.Task.get(), SIGNAL(finished()),
                this,            SLOT(onTaskFinished()), Qt::DirectConnection);

        m_tasks[data.Task.get()] = data;

        Task::submit(data.Task);
      }
    }

    validExecution = !validSegmentations.isEmpty();
  }

  if(validExecution && canExecute())
  {
    m_waitMutex.lock();
    m_condition.wait(&m_waitMutex);
    m_waitMutex.unlock();
  }

  if(isAborted())
  {
    abortTasks();
  }

  reportProgress(100);
}

//--------------------------------------------------------------------
void ApplyCountingFrame::abortTasks()
{
  for(auto task: m_tasks.keys())
  {
    disconnect(task, SIGNAL(progress(int, ApplySegmentationCountingFrame *)),
               this, SLOT(onTaskProgress(int, ApplySegmentationCountingFrame *)));

    disconnect(task, SIGNAL(finished()),
               this, SLOT(onTaskFinished()));

    if(!task->hasFinished())
    {
      task->abort();

      if(!task->thread()->wait(500))
      {
        task->thread()->terminate();
      }
    }
  }

  m_tasks.clear();
}

//--------------------------------------------------------------------
void ApplyCountingFrame::onTaskProgress(int value, ApplySegmentationCountingFrame *task)
{
  if(isAborted())
  {
    m_condition.wakeAll();
    return;
  }

  m_tasks[task].Progress = value;

  double progressValue = 0;
  for(auto data: m_tasks.values())
  {
    progressValue += data.Progress;
  }

  reportProgress(progressValue/Scheduler::maxRunningTasks());
}

//--------------------------------------------------------------------
void ApplyCountingFrame::onTaskFinished()
{
  if(isAborted())
  {
    m_condition.wakeAll();
    return;
  }

  bool finished = true;
  for(auto task: m_tasks.keys())
  {
    finished &= task->hasFinished();
  }

  if(finished)
  {
    m_condition.wakeAll();
  }
}

//--------------------------------------------------------------------
ApplySegmentationCountingFrame::ApplySegmentationCountingFrame(CountingFrame    *countingFrame,
                                                               SegmentationSList segmentations,
                                                               CoreFactory      *factory,
                                                               SchedulerSPtr     scheduler)
: Task           {scheduler}
, m_countingFrame{countingFrame}
, m_segmentations{segmentations}
, m_factory      {factory}
{
  setDescription(tr("Apply counting frame '%1'").arg(countingFrame->id()));
  setHidden(true);
}

//--------------------------------------------------------------------
void ApplySegmentationCountingFrame::run()
{
  if (!m_segmentations.isEmpty())
  {
    int oldProgress = 0;

    for (int i = 0; i < m_segmentations.size(); ++i)
    {
      if (!canExecute()) break;

      auto segmentation = m_segmentations.at(i);

      auto inclusionExtension = retrieveOrCreateSegmentationExtension<StereologicalInclusion>(segmentation, m_factory);
      inclusionExtension->addCountingFrame(m_countingFrame);
      inclusionExtension->evaluateCountingFrame(m_countingFrame);

      auto newProgress = (100 * i)/m_segmentations.size();

      if(oldProgress != newProgress)
      {
        oldProgress = newProgress;

        emit progress(oldProgress, this);
      }
    }
  }

  emit progress(100, this);
}
