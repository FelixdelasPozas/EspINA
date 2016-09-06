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

#ifndef ESPINA_CORE_MULTITASKING_TASKGROUPPROGESS_H
#define ESPINA_CORE_MULTITASKING_TASKGROUPPROGESS_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Task.h"

namespace ESPINA
{
  namespace Core
  {
    namespace MultiTasking
    {
      /** \class TaskGroupProgress
       * \brief Reports the progress of a set of tasks
       *
       */
      class EspinaCore_EXPORT TaskGroupProgress
      : public QObject
      {
        Q_OBJECT
      public:
        /** \brief Adds a task to the group of monitorized tasks.
         * \param[in] task task object.
         *
         */
        void showTaskProgress(TaskSPtr task);

      signals:
        void progress(int value);

      private slots:
        /** \brief Updates the progress value and emits the progress signal.
         *
         */
        void updateProgress();

        /** \brief Updates the group of monitorized tasks once a task has finished.
         *
         */
        void onTaskFinished();

      private:
        QList<TaskSPtr> m_tasks; /** group of tasks whose progress is being monitorized. */
        QMutex          m_mutex; /** protects task group.                                */
      };
    }
  }
}

#endif // ESPINA_CORE_MULTITASKING_TASKGROUPPROGESS_H
