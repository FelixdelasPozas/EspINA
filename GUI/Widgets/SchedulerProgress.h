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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Widgets/TaskProgress.h"
#include <Core/EspinaTypes.h>

// Qt
#include <QScrollArea>
#include <ui_SchedulerProgress.h>

namespace ESPINA {

  class EspinaGUI_EXPORT SchedulerProgress
  : public QWidget
  , private Ui::SchedulerProgress
  {
    Q_OBJECT
  public:
    struct Duplicated_Task_Exception{};

  public:
    /** brief SchedulerProgree class constructor.
     * \param[in] scheduler, scheduler smart pointer.
     * \param[in] parent, raw pointer of the QWidget parent of this one.
     */
    explicit SchedulerProgress(SchedulerSPtr   scheduler,
                               QWidget        *parent = nullptr,
                               Qt::WindowFlags f = 0);

    /** brief SchedulerProgress class virtual destructor.
     *
     */
    virtual ~SchedulerProgress();

  private slots:
  	/** brief Modifies the internal data when a task is added to the scheduler.
  	 *
  	 */
    void onTaskAdded(TaskSPtr task) throw (Duplicated_Task_Exception);

    /** brief Modifies the internal data when a task is removed from the scheduler.
  	 *
  	 */
    void onTaskRemoved(TaskSPtr task);

  	/** brief Shows/hides the individual task progress.
  	 * \param[in] visible, true to show the progress, false otherwise.
  	 *
  	 */
    void showTaskProgress(bool visible);

  	/** brief Updates the progress bar.
  	 *
  	 */
    void updateProgress();

  	/** brief Aborts a task.
  	 *
  	 * Aborting a task will remove it from the scheduler progress
  	 * but the task may remain executing until task abort is handled.
  	 *
  	 */
    void onProgressAborted();

  private:
  	/** brief Updates the values of the task progress notification widget.
  	 *
  	 */
    void updateNotificationWidget();

  private:
    SchedulerSPtr m_scheduler;

    QMap<TaskSPtr, TaskProgressSPtr> m_tasks;
    std::shared_ptr<QWidget>       m_notification;
    std::shared_ptr<QScrollArea>   m_notificationArea;

    int m_width;
    int m_height;
    unsigned int m_taskProgress;
    unsigned int m_taskTotal;
  };

  using SchedulerProgressSPtr = std::shared_ptr<SchedulerProgress>;
}

#endif // ESPINA_SCHEDULER_PROGRESS_H
