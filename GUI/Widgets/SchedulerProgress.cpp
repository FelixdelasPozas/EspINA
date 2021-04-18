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
#include <Core/Utils/EspinaException.h>

// Qt
#include <QLayout>
#include <QProgressBar>
#include <QLabel>
#include <QScrollBar>
#include <QDebug>
#include <QWidget>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SchedulerProgress::SchedulerProgress(SchedulerSPtr   scheduler,
                                     QWidget        *parent,
                                     Qt::WindowFlags f)
: QWidget           {parent, f}
, m_scheduler       {scheduler}
, m_notification    {new QWidget(this)}
, m_notificationArea{new QScrollArea(this)}
, m_width           {350}
, m_height          {400}
, m_taskProgress    {0}
, m_taskTotal       {0}
{
  setupUi(this);

  setVisible(false);

  setMinimumWidth(m_width+15);
  setMaximumWidth(m_width+15);

  m_notification->setLayout(new QVBoxLayout(m_notification));
  m_notification->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
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
  connect(m_stopTasks, SIGNAL(pressed()),
          this, SLOT(abortAllTasks()));
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
{
  m_mutex.lock();
  if (m_tasks.contains(task))
  {
    auto what    = QObject::tr("Attempt to add an existing task, task: %1").arg(task->description());
    auto details = QObject::tr("SchedulerProgress::onTaskAdded() -> Attempt to add an existing task, task: %1").arg(task->description());

    throw EspinaException(what, details);
  }

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
  if (visible && !m_tasks.isEmpty())
  {
    m_notificationArea->show();
    m_showTasks->setIcon(QIcon(":/espina/down.svg"));
    updateNotificationWidget();
  }
  else
  {
    m_notificationArea->hide();
    m_showTasks->setIcon(QIcon(":/espina/up.svg"));
  }
}

//------------------------------------------------------------------------
void SchedulerProgress::updateProgress()
{
  QMutexLocker lock(&m_mutex);

  int total = m_taskProgress;
  auto hasTasks = !m_tasks.isEmpty();

  if(hasTasks)
  {
    for(auto task : m_tasks.keys())
    {
      total += task->progress();
    }

    total = (total / m_taskTotal);
  }
  else
  {
    m_showTasks->setChecked(false);
  }

  m_progressBar->setValue(total);

  setVisible(hasTasks);
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

  auto size = m_notification->size();

  if(!size.isValid())
  {
    m_notificationArea->hide();
    m_showTasks->setChecked(false);
    m_showTasks->setIcon(QIcon(":/espina/up.svg"));
    return;
  }

  if (size.height() <= m_notificationArea->height())
  {
    m_notificationArea->setMaximumHeight(size.height());
    m_notificationArea->setMinimumHeight(size.height());
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

//------------------------------------------------------------------------
void SchedulerProgress::abortAllTasks()
{
  QMutexLocker lock(&m_mutex);

  for(auto widget: m_tasks.values())
  {
    widget->onCancel();
  }
}
