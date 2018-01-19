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
#include <Core/Types.h>

// Qt
#include <QScrollArea>
#include <ui_SchedulerProgress.h>

namespace ESPINA
{
  /** \class SchedulerProgress
   * \brief Implements a  widget that shows the progress of tasks running in the application sheduler.
   *
   */
  class EspinaGUI_EXPORT SchedulerProgress
  : public QWidget
  , private Ui::SchedulerProgress
  {
      Q_OBJECT
    public:
      /** \brief SchedulerProgree class constructor.
       * \param[in] scheduler application task scheduler.
       * \param[in] parent QWidget parent of this one.
       * \param[in] flags window flags.
       */
      explicit SchedulerProgress(SchedulerSPtr   scheduler,
                                 QWidget        *parent = nullptr,
                                 Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief SchedulerProgress class virtual destructor.
       *
       */
      virtual ~SchedulerProgress();

    private slots:
      /** \brief Modifies the internal data when a task is added to the scheduler.
       *
       */
      void onTaskAdded(TaskSPtr task);

      /** \brief Modifies the internal data when a task is removed from the scheduler.
       *
       */
      void onTaskRemoved(TaskSPtr task);

      /** \brief Shows/hides the individual task progress.
       * \param[in] visible, true to show the progress, false otherwise.
       *
       */
      void showTaskProgress(bool visible);

      /** \brief Updates the progress bar.
       *
       */
      void updateProgress();

      /** \brief Aborts a task.
       *
       * Aborting a task will remove it from the scheduler progress
       * but the task may remain executing until task abort is handled.
       *
       */
      void onProgressAborted();

    private:
      /** \brief Updates the values of the task progress notification widget.
       *
       */
      void updateNotificationWidget();

    private:
      SchedulerSPtr m_scheduler;

      QMutex                           m_mutex;             /** task data mutex.               */
      QMap<TaskSPtr, TaskProgressSPtr> m_tasks;             /** registered tasks to watch.     */
      QWidget                         *m_notification;      /** notification widget.           */
      std::shared_ptr<QScrollArea>     m_notificationArea;  /** notification area.             */
      int                              m_width;             /** notification area width.       */
      int                              m_height;            /** notification area max height.  */
      unsigned int                     m_taskProgress;      /** last task progress on removal. */
      unsigned int                     m_taskTotal;         /** number of watched tasks.       */
  };

  using SchedulerProgressSPtr = std::shared_ptr<SchedulerProgress>;
}

#endif // ESPINA_SCHEDULER_PROGRESS_H
