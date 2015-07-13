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

#include "TaskGroupProgress.h"

#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::MultiTasking;

//----------------------------------------------------------------------------
void TaskGroupProgress::showTaskProgress(TaskSPtr task)
{
  if (!m_tasks.contains(task))
  {
    m_tasks << task;

    connect(task.get(), SIGNAL(progress(int)),
            this,       SLOT(updateProgress()));

    connect(task.get(), SIGNAL(finished()),
            this,       SLOT(onTaskFinished()));
  }
  else
  {
    qDebug() << "Already containing task";
  }
}

//----------------------------------------------------------------------------
void TaskGroupProgress::updateProgress()
{
  int total = 0;

  if (!m_tasks.isEmpty())
  {
    for(auto task : m_tasks)
    {
      total += task->progress();
    }

    total = total / m_tasks.size();
  }

  emit progress(total);
}

//----------------------------------------------------------------------------
void TaskGroupProgress::onTaskFinished()
{
  auto finishedTask = static_cast<TaskPtr>(sender());

  qDebug() << "Task finished";
  int i      = 0;
  bool found = false;

  while (i < m_tasks.size() && !found)
  {
    found = m_tasks[i].get() == finishedTask;

    if (!found)
    {
      ++i;
    }
  }

  Q_ASSERT(found);

  m_tasks.removeAt(i);

  updateProgress();
}

