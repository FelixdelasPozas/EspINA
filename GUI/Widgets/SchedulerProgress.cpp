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
#include <QDebug>
using namespace EspINA;

//------------------------------------------------------------------------
SchedulerProgress::SchedulerProgress(SchedulerSPtr   scheduler,
                                     QWidget        *parent,
                                     Qt::WindowFlags f)
: QWidget(parent, f)
, m_scheduler(scheduler)
, m_notification{new QWidget(0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint)}
, m_width{350}
{
  setupUi(this);

  setVisible(false);

  m_notification->setLayout(new QVBoxLayout(m_notification.get()));
  m_notification->setMinimumWidth(m_width);
  m_notification->setMaximumWidth(m_width);


  qRegisterMetaType<TaskSPtr>("TaskSPtr");

  connect(m_scheduler.get(), SIGNAL(taskAdded(TaskSPtr)),
          this, SLOT(onTaskAdded(TaskSPtr)));
  connect(m_scheduler.get(), SIGNAL(taskRemoved(TaskSPtr)),
          this, SLOT(onTaskRemoved(TaskSPtr)));
  connect(m_showTasks, SIGNAL(toggled(bool)),
          this, SLOT(showTaskProgress(bool)));
}

//------------------------------------------------------------------------
SchedulerProgress::~SchedulerProgress()
{
  for(auto task: m_tasks.keys())
  {
    m_tasks[task] = nullptr;
    m_tasks.remove(task);
  }
}

//------------------------------------------------------------------------
void SchedulerProgress::onTaskAdded(TaskSPtr task)
throw (Duplicated_Task_Exception)
{
  if (m_tasks.contains(task)) throw Duplicated_Task_Exception();

  TaskProgressSPtr taskProgress{new TaskProgress(task)};

  m_tasks[task] = taskProgress;
  connect(taskProgress.get(), SIGNAL(aborted()),
          this, SLOT(onProgressAborted()));

  m_notification->layout()->addWidget(m_tasks[task].get());
  taskProgress->setHidden(true);

  connect(task.get(), SIGNAL(progress(int)),
          this, SLOT(updateProgress()));

  updateNotificationWidget();
}

//------------------------------------------------------------------------
void SchedulerProgress::onTaskRemoved(TaskSPtr task)
{
  if (m_tasks.contains(task))
  {
    disconnect(task.get(), SIGNAL(progress(int)),
               this, SLOT(updateProgress()));

    m_tasks[task]->setParent(0); // In case the notification are is open
    m_tasks.remove(task);
  }

  updateNotificationWidget();
  updateProgress();
}

//------------------------------------------------------------------------
void SchedulerProgress::showTaskProgress(bool visible)
{
  if (visible)
  {
    m_notification->show();
    updateNotificationWidget();
    m_showTasks->setIcon(QIcon(":/espina/down.png"));
  } else 
  {
    m_notification->hide();
    m_showTasks->setIcon(QIcon(":/espina/up.png"));
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

  setVisible(0 != total || m_tasks.size() != 0);
}

//------------------------------------------------------------------------
void SchedulerProgress::onProgressAborted()
{
  auto taskProgress = dynamic_cast<TaskProgress *>(sender());

  onTaskRemoved(taskProgress->task());
}


//------------------------------------------------------------------------
void SchedulerProgress::updateNotificationWidget()
{
  for(auto task: m_tasks.keys())
    if(task->isRunning() && !task->isHidden())
      m_tasks[task]->setHidden(false);
    else
      m_tasks[task]->setHidden(true);

  m_notification->adjustSize();

  int xShift = m_showTasks->width() - m_width;
  int yShift = -m_notification->height();

  m_notification->move(mapToGlobal(m_showTasks->pos()+QPoint(xShift, yShift)));
}
