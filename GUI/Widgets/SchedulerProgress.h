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

#ifndef ESPINA_SCHEDULER_PROGRESS_H
#define ESPINA_SCHEDULER_PROGRESS_H

#include <Core/EspinaTypes.h>
#include "GUI/Widgets/TaskProgress.h"

#include <ui_SchedulerProgress.h>

namespace ESPINA {

  class SchedulerProgress 
  : public QWidget
  , private Ui::SchedulerProgress
  {
    Q_OBJECT
  public:
    struct Duplicated_Task_Exception{};

  public:
    explicit SchedulerProgress(SchedulerSPtr   scheduler,
                               QWidget        *parent = 0,
                               Qt::WindowFlags f = 0);

    virtual ~SchedulerProgress();

  private slots:
    void onTaskAdded  (TaskSPtr task) throw (Duplicated_Task_Exception);

    void onTaskRemoved(TaskSPtr task);

    void showTaskProgress(bool visible);

    void updateProgress();

    // Aborting a task will remove it from the scheduler progress
    // but the task may remain executing until task abort is handled
    void onProgressAborted();

  private:
    void updateNotificationWidget();

  private:
    SchedulerSPtr m_scheduler;

    QMap<TaskSPtr, TaskProgressSPtr> m_tasks;
    std::shared_ptr<QWidget>       m_notification;

    int m_width;
  };

  using SchedulerProgressSPtr = std::shared_ptr<SchedulerProgress>;
}

#endif // ESPINA_SCHEDULER_PROGRESS_H
