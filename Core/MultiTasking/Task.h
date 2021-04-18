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

#ifndef ESPINA_WORKER_H
#define ESPINA_WORKER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Types.h>

// Qt
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QStringList>

// C++
#include <cstdint>
#include <atomic>

namespace ESPINA
{
  enum class Priority: std::int8_t { VERY_LOW = 0, LOW = 1, NORMAL = 2, HIGH = 3, VERY_HIGH = 4 };

  class Task;
  using TaskPtr  = Task *;
  using TaskSPtr = std::shared_ptr<Task>;

  /** \class Task
   * \brief Espina threaded task base class.
   *
   */
  class EspinaCore_EXPORT Task
  : public QObject
  {
    Q_OBJECT
  public:
    typedef unsigned int Id;
  public:
    /** \brief Task class constructor.
     * \param[in] scheduler scheduler smart pointer.
     *
     */
    explicit Task(SchedulerSPtr scheduler);

    /** \brief Task class virtual destructor.
     *
     */
    virtual ~Task();

    /** \brief Sets task id.
     * \param[in] id of the task
     *
     */
    void setId(Id id)
    { m_id = id;}

    /** \brief Returns task id.
     *
     */
    Id id() const
    { return m_id;}

    /** \brief Sets task description.
     * \param[in] description of the task
     *
     */
    void setDescription(const QString& description)
    {
      QWriteLocker lock(&m_descriptionLock);
      m_description = description;
    }

    /** \brief Returns task description.
     *
     */
    QString description() const
    {
      QReadLocker lock(&m_descriptionLock);
      return m_description;
    }

    /** \brief Pause worker execution
     *
     *  It is up to the worker to implement the mechanism needed to interrupt its execution
     *  Paused workers will ignore signals on its slots
     */
    virtual void pause();

    /** \brief Resume worker execution
     *
     *  It is up to the worker to implement the mechanism needed to resume its execution
     */
    virtual void resume();

    /** \brief Returns true if the task is running but has been signaled to pause.
     *
     */
    virtual bool isPendingPause() const;

    void restart();

    /** \brief Returns true if the task is running.
     *
     */
    virtual bool isRunning() const;

    /** \brief Returns true if the task has been aborted.
     *
     */
    bool isAborted() const;

    /** \brief Returns true if the task has finished its execution.
     *
     */
    bool hasFinished() const;

    /** \brief Returns true if the task encountered any error in it's execution.
     *         Override it if your task can or should report errors.
     *
     */
    virtual bool hasErrors() const
    { return false; }

    /** \brief Returns the list of errors the task encountered during it's execution.
     *         Override it if your task can or should report errors.
     *
     */
    virtual const QStringList errors() const
    { return QStringList(); }

    /** \brief Returns task priority.
     *
     */
    Priority priority() const
    { return m_priority; }

    /** \brief Sets the task as hidden.
     * \param[in] hidden true to set as hidden, false otherwise.
     *
     * Scheduler won't signal any action regarding hidden tasks.
     *
     */
    void setHidden(bool hidden)
    { m_hidden = hidden; }

    /** \brief Returns true if the task is a hidden task.
     *
     */
    bool isHidden() const
    { return m_hidden; }

    /** \brief Returns true if the task is paused.
     *
     */
    bool isPaused() const
    { return m_isPaused; }

    int progress() const
    { return m_progress; }

  public slots:
    /** \brief Emits progress signal.
     *
     * NOTE: Need to be public so we can reuse itkProgressReporters.
     */
    void reportProgress(int value);

    /** \brief Sets task priority.
     * \param[in] value priority level.
     *
     */
    void setPriority(Priority value);

    /** \brief Notify the scheduler this task is ready to be executed.
     *
     *  From this time, it is responsibility of the scheduler to active this task.
     */
    static void submit(TaskSPtr task);

    /** \brief Abort task execution.
     *
     *  It is up to the task to implement the mechanism needed to pause, resume or abort its execution
     */
    void abort();

  protected:
    /** \brief Returns true if the task has not been signaled to abort or pause.
     *
     */
    bool canExecute();

    /** \brief Returns true if the task is sheduled to start from the beginning aborting the current execution.
     *
     */
    bool needsRestart() const;

    /** \brief Sets the task as finished.
     * \param[in] value finished value.
     */
    void setFinished(bool value);

  private:
    /** \brief Executes the task.
     *
     */
    virtual void run() = 0;

    /** \brief Pauses the task from the scheduler.
     *
     */
    void dispatcherPause();

    /** \brief Resumes the task from the scheduler.
     *
     */
    void dispatcherResume();

    /** \brief Returns true if the scheduler paused the task.
     *
     */
    bool isDispatcherPaused();

    /** \brief Helper method to set some values before execution.
     *
     */
    void prepareToRun();

    /** \brief Method to be executed when a task is aborted
     *
     *  Overload this method when your task needs to do some processing when it is aborted
     */
    virtual void onAbort()
    {}

  private slots:
    /** \brief Helper method to set some values after execution.
     *
     */
    void runWrapper();

    /** \brief Starts the thread.
     *
     */
    void startThreadExecution();

    /** \brief Helper method to do actions once the run() execution of the thread has finished.
     *
     *
     */
    void onTaskFinished();

  private:
    /** \brief Returns the the execution to the originating thread and destroys the thread.
     *
     */
    void finishThreadExecution();

    /** \brief Returns true if the run() method is running on an independent (separated) thread.
     *
     */
    bool isExecutingOnThread() const;

  signals:
    void progress(int);
    void resumed();
    void paused();
    void finished();
    void aborted();

  protected:
    SchedulerSPtr m_scheduler; /** application task scheduler. */

  private:
    QThread *m_thread;          /** thread that launches the task.                                  */
    QThread *m_executingThread; /** thread where the task is executed.                              */
    bool     m_submitted;       /** true if task has been submitted to the scheduler for execution. */
    QMutex   m_submissionMutex; /** submission data protection mutex.                               */

    Priority m_priority;

    std::atomic<bool> m_isRunning;        /** true if the task is on the 'running' state, false otherwise.                        */
    std::atomic<bool> m_pendingPause;     /** true if the task is going to be paused by the scheduler, false otherwise.           */
    std::atomic<bool> m_pendingUserPause; /** true if the task is going to be paused because the user paused it, false otherwise. */
    std::atomic<bool> m_isAborted;        /** true if the task has been aborted, false otherwise.                                 */
    std::atomic<bool> m_hasFinished;      /** true if the method run() has finished its execution, false otherwise.               */
    std::atomic<bool> m_isPaused;         /** true if the task is paused (by scheduler or by user), false otherwise.              */
    std::atomic<bool> m_needsRestart;     /** true if the task needs to be restarted, false otherwise.                            */

    Id                m_id;               /** task identifier.                                                                    */
    bool              m_hidden;           /** true to hide the task to the user interface, false to make it public.               */
    QMutex            m_mutex;            /** data protection mutex.                                                              */
    QWaitCondition    m_pauseCondition;   /** wait condition for the paused state.                                                */

    QString           m_description;      /** task description text.                                                              */
    std::atomic<int>  m_progress;         /** task current progress value in [0-100]                                              */

    mutable QReadWriteLock m_descriptionLock; /** lock for accessing the description data. */

    friend class Scheduler;
  };
}

#endif // ESPINA_WORKER_H
