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

#ifndef ESPINA_SCHEDULER_H
#define ESPINA_SCHEDULER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Task.h"

// Qt
#include <QMap>

// C++
#include <atomic>

namespace ESPINA
{
  /** \class Scheduler
   * \brief Task scheduler.
   *
   */
  class EspinaCore_EXPORT Scheduler
  : public QObject
  {
    Q_OBJECT

// TODO: @felix re-schedule executing tasks to lower the max running tasks number to max CPU threads.
//    struct ScheduledTask
//    {
//      static const unsigned MAX_CICLES = 100;
//
//      explicit ScheduledTask(TaskSPtr task)
//      : Task(task)
//      , Cicles(MAX_CICLES)
//      {}
//
//      TaskSPtr Task;
//      unsigned Cicles;
//
//      bool consumeCicle()
//      {
//        return --Cicles > 0;
//      }
//
//      void restoreCicles()
//      { Cicles = MAX_CICLES; }
//
//      bool operator==(const ScheduledTask &rhs)
//      { return Task == rhs.Task; }
//    };

    using ScheduledTask = TaskSPtr;

    /** \class TaskQueue
     * \brief List of tasks ordered by task id.
     *
     */
    class TaskQueue
    : public QList<ScheduledTask>
    {
      public:
        /** \brief Inserts the given task in the list in order.
         *
         */
        void orderedInsert(TaskSPtr task);
    };


  public:
    static const unsigned int MAX_TASKS = 15; /** max number of tasks. */

    /** \brief Scheduler class constructor.
     * \param[in] period interval for scheduling tasks.
     * \param[in] parent raw pointer of the parent of this object.
     *
     */
    explicit Scheduler(int period/*ns*/, QObject* parent = 0);

    /** \brief Scheduler class destructor.
     *
     */
    virtual ~Scheduler();

    /** \brief Adds a task to the task list.
     * \param[in] task task smart pointer.
     *
     */
    void addTask(TaskSPtr task);

    /** \brief Aborts all tasks currently in the task list and in the insertion list. Clears all lists.
     *
     */
    void abort();

    /** \brief Changes a task priority.
     * \param[in] task task raw pointer.
     *
     */
    void changePriority(TaskPtr task, Priority prevPriority);

    /** \brief Changes a task priority.
     * \param[in] task task smart pointer.
     *
     */
    void changePriority(TaskSPtr task, Priority prevPriority);

    /** \brief Returns the maximum number of threads of the system.
     *
     */
    static unsigned int maxRunningTasks();

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
    /** \brief Helper method to insert recently added tasks into the scheduler.
     *
     */
    void proccessTaskInsertion();

    /** \brief Changes the currently running tasks based on the priorities.
     *
     */
    void reschedule();

    /** \brief Helper method to process the task's priority changes.
     *
     */
    void proccessPriorityChanges();

    /** \brief Helper re-scheduling method. Currently disabled.
     *
     */
    void roundRobinShift();

    /** \brief Removes a task from the task list.
     * \param[in] task task smart pointer.
     *
     */
    void removeTask(Priority priority, TaskSPtr task);

    /** \brief Returns true if the given task can execute.
     * \param[in] task task smart pointer.
     *
     */
    bool canExecute(TaskSPtr task) const;

    /** \brief Helper method to print the state of the given task in std::cout.
     * \param[in] task task smart pointer.
     *
     */
    void printState(TaskSPtr task) const;

  private:
    int                       m_period;             /** schduler executing period.                                 */
    QMutex                    m_insertionMutex;     /** mutex to protect insetion list.                            */
    QList<TaskSPtr>           m_insertionBuffer;    /** task insertion list, not currently in execution.           */
    QMutex                    m_priorityMutex;      /** mutex to protect tasks priorities.                         */
    QMap<TaskPtr, Priority>   m_priorityBuffer;     /** maps task<->priority.                                      */
    QMap<Priority, TaskQueue> m_scheduledTasks;     /** maps priority<->tasks.                                     */
    Task::Id                  m_lastId;             /** id of last added task.                                     */
    unsigned int              m_maxNumRunningTasks; /** maximum number of running tasks.                           */
    mutable QMutex            m_mutex;              /** scheduler data mutex.                                      */
    std::atomic<bool>         m_abort;              /** true to abort the schduler and finish, false otherwise.    */
    QMutex                    m_waitMutex;          /** wait condition mutex.                                      */
    QWaitCondition            m_wait;               /** wait condition to correctly finish and free all resources. */
  };
}

#endif // ESPINA_SCHEDULER_H
