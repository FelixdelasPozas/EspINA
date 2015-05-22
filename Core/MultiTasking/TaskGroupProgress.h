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

#include "Task.h"

namespace ESPINA {
  namespace Core {
    namespace MultiTasking {

      /** \brief Reports the progress of a set of tasks
       *
       */
      class TaskGroupProgress
      : public QObject
      {
        Q_OBJECT
      public:
        void showTaskProgress(TaskSPtr task);

      signals:
        void progress(int value);

      private slots:
        void updateProgress();

        void onTaskFinished();

      private:
        QList<TaskSPtr> m_tasks;
      };
    }
  }
}

#endif // ESPINA_CORE_MULTITASKING_TASKGROUPPROGESS_H
