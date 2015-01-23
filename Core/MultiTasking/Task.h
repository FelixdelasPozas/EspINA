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
#include <Core/EspinaTypes.h>

// Qt
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>

// C++
#include <cstdint>

namespace ESPINA {

  enum class Priority: std::int8_t { VERY_LOW = 0, LOW = 1, NORMAL = 2, HIGH = 3, VERY_HIGH = 4 };

  class Task;
  using TaskPtr  = Task *;
  using TaskSPtr = std::shared_ptr<Task>;

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

    /** \brief Returns true if the task is running.
     *
     */
    bool isRunning() const;

    /** \brief Returns true if the task has been aborted.
     *
     */
    bool isAborted() const
    { return m_isAborted; }

    /** \brief Returns true if the task has finished its execution.
     *
     */
    bool hasFinished() const
    { return m_hasFinished; }

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

    /** \brief The task is waiting waiting to another process to finish.
     *
     */
    bool isWaiting() const {return m_isWaiting; }

    /** \brief Emits progress signal.
     *
     * NOTE: Need to be public so we can reuse itkProgressReporters.
     */
    void reportProgress(int value)
    { emit progress(value); }


  public slots:
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

    /** \brief Change the waiting state of the task.
     *
     *  Usually a task should change its state before executing potentially blocking calls
     *  and restore it after the call itself so the scheduler may execute other pending tasks
     */
    void setWaiting(bool value)
    { m_isWaiting = value; }

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
    /** \brief Starts the thread.
     *
     */
    void start();

    /** \brief Helper method to set some values after execution.
     *
     */
    void runWrapper();

  signals:
    void progress(int);
    void resumed();
    void paused();
    void finished();

  protected:
    SchedulerSPtr m_scheduler;

  private:
    Priority m_priority;

    bool m_isRunning;
    bool m_pendingPause;
    bool m_pendingUserPause;
    bool m_isAborted;
    bool m_hasFinished;
    bool m_isPaused;
    bool m_isWaiting;
    bool m_isThreadAttached;

    Id   m_id;
    bool m_hidden;

    QMutex         m_mutex;
    mutable QReadWriteLock m_descriptionLock;
    QWaitCondition m_pauseCondition;

    QString  m_description;

    friend class Scheduler;
  };
}

#endif // ESPINA_WORKER_H
