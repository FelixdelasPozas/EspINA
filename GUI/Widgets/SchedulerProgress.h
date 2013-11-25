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

#ifndef ESPINA_SCHEDULER_PROGRESS_H
#define ESPINA_SCHEDULER_PROGRESS_H

#include <Core/EspinaTypes.h>
#include "GUI/Widgets/TaskProgress.h"

#include <ui_SchedulerProgress.h>

namespace EspINA {

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

  private slots:
    void onTaskAdded  (Task *task) throw (Duplicated_Task_Exception);
    void onTaskRemoved(Task *task);
    void showTaskProgress(bool visible);
    void updateProgress();

  private:
    void updateNotificationWidget();

  private:
    SchedulerSPtr m_scheduler;

    QMap<Task *, TaskProgressSPtr> m_tasks;
    std::shared_ptr<QWidget>       m_notification;

    int m_width;
  };

  using SchedulerProgressSPtr = std::shared_ptr<SchedulerProgress>;
}

#endif // ESPINA_SCHEDULER_PROGRESS_H