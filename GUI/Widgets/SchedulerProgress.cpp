/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "SchedulerProgress.h"
#include "TaskProgress.h"
#include <Core/MultiTasking/Task.h>
#include <Core/MultiTasking/Scheduler.h>

#include <QLayout>
#include <QProgressBar>
#include <QLabel>

using namespace EspINA;

//------------------------------------------------------------------------
SchedulerProgress::SchedulerProgress(SchedulerSPtr   scheduler,
                                     QWidget        *parent,
                                     Qt::WindowFlags f)
: QWidget(parent, f)
, m_scheduler(scheduler)
, m_notification{new QWidget(0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint)}
{
  setupUi(this);

  setVisible(false);

  m_notification->setLayout(new QVBoxLayout(m_notification.get()));
  m_notification->setMinimumWidth(200);
  m_notification->setMaximumWidth(200);

  connect(m_scheduler.get(), SIGNAL(taskAdded(Task*)),
          this, SLOT(onTaskAdded(Task*)));
  connect(m_scheduler.get(), SIGNAL(taskRemoved(Task*)),
          this, SLOT(onTaskRemoved(Task*)));
  connect(m_showTasks, SIGNAL(toggled(bool)),
          this, SLOT(showTaskProgress(bool)));
}

//------------------------------------------------------------------------
void SchedulerProgress::onTaskAdded(Task *task)
throw (Duplicated_Task_Exception)
{
  if (m_tasks.contains(task)) throw Duplicated_Task_Exception();

  m_tasks[task] = TaskProgressSPtr{new TaskProgress(task)};

  m_notification->layout()->addWidget(m_tasks[task].get());

  connect(task, SIGNAL(progress(int)),
          this, SLOT(updateProgress()));
}

//------------------------------------------------------------------------
void SchedulerProgress::onTaskRemoved(Task *task)
{
  if (m_tasks.contains(task))
  {
    disconnect(task, SIGNAL(progress(int)),
               this, SLOT(updateProgress()));

    m_tasks[task]->setParent(0); // In case the notification are is open
    m_tasks.remove(task);

    updateProgress();
  }
}

//------------------------------------------------------------------------
void SchedulerProgress::showTaskProgress(bool visible)
{
  if (visible)
  {
    int xShift = 0;//m_showTasks->width() - m_notification->sizeHint().width();
    int yShift = -m_notification->sizeHint().height();

    m_notification->move(mapToGlobal(m_showTasks->pos()+QPoint(xShift, yShift)));
    m_notification->show();
  } else 
  {
    m_notification->hide();
  }
}

//------------------------------------------------------------------------
void SchedulerProgress::updateProgress()
{
  int total = 0;

  for(TaskProgressSPtr task : m_tasks)
  {
    total += task->progress();
  }

  if (total > 0)
  {
    total = total / m_tasks.count();
  } else
  {
    m_showTasks->setChecked(false);
  }

  m_progressBar->setValue(total);

  setVisible(0 != total);
}

