/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ESPINA_DISPATCHER_H
#define ESPINA_DISPATCHER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Task.h"

namespace ESPINA {

  class TaskQueue
  : public QList<TaskSPtr>
  {
		public:
			void orderedInsert(TaskSPtr worker);
  };

  class EspinaCore_EXPORT Scheduler
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief Scheduler class constructor.
     * \param[in] period, interval for scheduling tasks.
     * \param[in] parent, raw pointer of the parent of this object.
     *
     */
    explicit Scheduler(int period/*ns*/, QObject* parent = 0);

    /** \brief Scheduler class destructor.
     *
     */
    virtual ~Scheduler();

    /** \brief Adds a task to the task list.
     * \param[in] task, task smart pointer.
     *
     */
    void addTask(TaskSPtr task);

    /** \brief Removes a task from the task list.
     * \param[in] task, task smart pointer.
     *
     */
    void removeTask(TaskSPtr task);

    /** \brief Aborts all tasks currently in the task list.
     *
     */
    void abortExecutingTasks();

    /** \brief Changes a task priority.
     * \param[in] task, task raw pointer.
     *
     */
    void changePriority(TaskPtr task, Priority prevPriority);

    /** \brief Changes a task priority.
     * \param[in] task, task smart pointer.
     *
     */
    void changePriority(TaskSPtr task, Priority prevPriority);

    /** \brief Returns the number of task currently in the task list.
     *
     */
    unsigned int numberOfTasks() const;

  public slots:
		/** \brief Starts the scheduler.
		 *
		 */
    void scheduleTasks();

  signals:
    void taskAdded(TaskSPtr);
    void taskRemoved(TaskSPtr);

  private:
    int m_period;

    std::map<Priority, TaskQueue> m_runningTasks;

    Task::Id m_lastId;

    int            m_maxNumRunningThreads;
    mutable QMutex m_mutex;
    bool           m_abort;
  };
}

#endif // ESPINA_DISPATCHER_H
