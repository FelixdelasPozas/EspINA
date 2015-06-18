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

// ESPINA
#include "SchedulerProgress.h"
#include "TaskProgress.h"
#include <Core/MultiTasking/Task.h>
#include <Core/MultiTasking/Scheduler.h>

// Qt
#include <QLayout>
#include <QProgressBar>
#include <QLabel>
#include <QScrollBar>
#include <QDebug>

using namespace ESPINA;

//------------------------------------------------------------------------
SchedulerProgress::SchedulerProgress(SchedulerSPtr   scheduler,
                                     QWidget        *parent,
                                     Qt::WindowFlags f)
: QWidget           {parent, f}
, m_scheduler       {scheduler}
, m_notification    {new QWidget(nullptr)}
, m_notificationArea{new QScrollArea(nullptr)}
, m_width           {350}
, m_height          {400}
, m_taskProgress    {0}
, m_taskTotal       {0}
{
  setupUi(this);

  setVisible(false);

  m_notification->setLayout(new QVBoxLayout(m_notification));
  m_notification->setMinimumWidth(m_width);
  m_notification->setMaximumWidth(m_width);

  m_notificationArea->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  m_notificationArea->setWidget(m_notification);
  m_notificationArea->setMaximumWidth(m_width+15);
  m_notificationArea->setMinimumWidth(m_width+15);
  m_notificationArea->setMaximumHeight(m_height);
  m_notificationArea->setMinimumHeight(m_height);
  m_notificationArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_notificationArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  qRegisterMetaType<TaskSPtr>("TaskSPtr");

  // NOTE: Need to be queued to prevent signal reordering
  connect(m_scheduler.get(), SIGNAL(taskAdded(TaskSPtr)),
          this, SLOT(onTaskAdded(TaskSPtr)), Qt::QueuedConnection);
  connect(m_scheduler.get(), SIGNAL(taskRemoved(TaskSPtr)),
          this, SLOT(onTaskRemoved(TaskSPtr)), Qt::QueuedConnection);
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
  m_mutex.lock();
  if (m_tasks.contains(task)) throw Duplicated_Task_Exception();

  ++m_taskTotal;

  auto taskProgress = std::make_shared<TaskProgress>(task);

  m_tasks[task] = taskProgress;
  m_mutex.unlock();

  connect(taskProgress.get(), SIGNAL(aborted()),
          this,               SLOT(onProgressAborted()));

  m_notification->layout()->addWidget(m_tasks[task].get());

  connect(task.get(), SIGNAL(progress(int)),
          this,       SLOT(updateProgress()));

  updateNotificationWidget();
}

//------------------------------------------------------------------------
void SchedulerProgress::onTaskRemoved(TaskSPtr task)
{
  m_mutex.lock();
  Q_ASSERT(m_tasks.contains(task));

  m_notification->layout()->removeWidget(m_tasks[task].get());

  m_tasks[task]->setParent(0); // In case the notification area is open
  m_tasks.remove(task);

  if (m_tasks.isEmpty())
  {
    m_taskProgress = 0;
    m_taskTotal    = 0;
  }
  else
  {
    m_taskProgress += 100;
  }

  m_mutex.unlock();

  disconnect(task.get(), SIGNAL(progress(int)),
             this,       SLOT(updateProgress()));

  updateNotificationWidget();
  updateProgress();
}

//------------------------------------------------------------------------
void SchedulerProgress::showTaskProgress(bool visible)
{
  if (visible)
  {
    m_notificationArea->show();
    updateNotificationWidget();
    m_showTasks->setIcon(QIcon(":/espina/down.png"));
  } else
  {
    m_notificationArea->hide();
    m_showTasks->setIcon(QIcon(":/espina/up.png"));
  }
}

//------------------------------------------------------------------------
void SchedulerProgress::updateProgress()
{
  QMutexLocker lock(&m_mutex);

  int total = m_taskProgress;

  for(TaskProgressSPtr task : m_tasks)
  {
    total += task->progress();
  }

  if (total > 0)
  {
    total = (total / m_taskTotal);
  }
  else
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
  taskProgress->m_cancelButton->setEnabled(false);
  taskProgress->task()->abort();
}

//------------------------------------------------------------------------
void SchedulerProgress::updateNotificationWidget()
{
  m_notification->adjustSize();

  auto wHeight = m_notification->height();

  if (wHeight <= m_notificationArea->height())
  {
    m_notificationArea->setMaximumHeight(wHeight);
    m_notificationArea->setMinimumHeight(wHeight);
    m_notificationArea->verticalScrollBar()->hide();
  }
  else
  {
    m_notificationArea->setMaximumHeight(m_height);
    m_notificationArea->setMinimumHeight(m_height);
    m_notificationArea->verticalScrollBar()->show();
  }
  m_notificationArea->adjustSize();

  int xShift = m_showTasks->width() - m_width-15;
  int yShift = -m_notificationArea->height();

  m_notificationArea->move(mapToGlobal(m_showTasks->pos()+QPoint(xShift, yShift)));
}
