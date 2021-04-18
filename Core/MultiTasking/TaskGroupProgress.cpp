/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "TaskGroupProgress.h"

using namespace ESPINA;
using namespace ESPINA::Core::MultiTasking;

//----------------------------------------------------------------------------
void TaskGroupProgress::showTaskProgress(TaskSPtr task)
{
  {
    QMutexLocker lock(&m_mutex);

    if(m_tasks.contains(task)) return;

    m_tasks << task;
  }

  connect(task.get(), SIGNAL(progress(int)),
          this,       SLOT(updateProgress()));

  connect(task.get(), SIGNAL(finished()),
          this,       SLOT(onTaskFinished()));

  connect(task.get(), SIGNAL(aborted()),
          this,       SLOT(onTaskFinished()));
}

//----------------------------------------------------------------------------
void TaskGroupProgress::updateProgress()
{
  int total   = 0;

  {
    QMutexLocker lock(&m_mutex);

    if (!m_tasks.isEmpty())
    {
      int partial = 0;

      for(auto task : m_tasks) partial += task->progress();

      total = (m_finished*100 + partial) / (m_tasks.size() + m_finished);
    }
    else
    {
      m_finished = 0;
      total = 100;
    }
  }

  emit progress(total);
}

//----------------------------------------------------------------------------
void TaskGroupProgress::onTaskFinished()
{
  auto finishedTask = static_cast<TaskPtr>(sender());
  if(!finishedTask) return;

  disconnect(finishedTask, SIGNAL(progress(int)),
             this,         SLOT(updateProgress()));

  disconnect(finishedTask, SIGNAL(finished()),
             this,         SLOT(onTaskFinished()));

  disconnect(finishedTask, SIGNAL(aborted()),
             this,         SLOT(onTaskFinished()));

  {
    QMutexLocker lock(&m_mutex);

    int  i     = 0;
    bool found = false;

    while (i < m_tasks.size() && !found)
    {
      found = m_tasks[i].get() == finishedTask;

      if (!found)
      {
        ++i;
      }
    }

    if(!found) return;

    m_tasks.removeAt(i);
    ++m_finished;
  }

  updateProgress();
}

