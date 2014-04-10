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

#include <QObject>

#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <Core/EspinaTypes.h>

namespace EspINA {

  enum Priority { VERY_LOW, LOW, NORMAL, HIGH, VERY_HIGHT };

  class Task;
  using TaskPtr  = Task *;
  using TaskSPtr = std::shared_ptr<Task>;

  class Task 
  : public QObject
  {
    Q_OBJECT
  public:
    typedef unsigned int Id;
  public:
    explicit Task(SchedulerSPtr scheduler);
    virtual ~Task();

    void setId(Id id) { m_id = id;}

    Id id() const { return m_id;}

    void setDescription(const QString& description)
    {
      QWriteLocker lock(&m_descriptionLock);
      m_description = description;
    }

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

    virtual bool isPendingPause() const;

    bool isRunning() const {return m_isThreadAttached && !isPendingPause();}

    bool isAborted() const { return m_isAborted; }

    bool hasFinished() const { return m_hasFinished; }

    int priority() const { return m_priority; }

    void setHidden(bool hidden) { m_hidden = hidden; }

    bool isHidden() const { return m_hidden; }

    bool isPaused() const { return m_isPaused; }

    /** \brief The task is waiting waiting to another process to finished
     *
     */
    bool isWaiting() const {return m_isWaiting; }

  public slots:
    void setPriority(const int value);

    /** \brief Notify the scheduler this task is ready to be executed
     * 
     *  From this time, it is reponsability of the scheduler to active this task
     */
    static void submit(TaskSPtr task);

    /** \brief Abort task execution
     * 
     *  It is up to the task to implement the mechanism needed to pause, resume or abort its execution
     */
    void abort();

  protected:
    bool canExecute();

    virtual void run() = 0;

    /** \brief Change the waiting state of the task
     *
     *  Usually a task should change its state before executing potentially blocking calls
     *  and restore it after the call itself so the scheduler may execute other pending tasks
     */
    void setWaiting(bool value)
    { m_isWaiting = value; }

    void setFinished(bool value)
    {
      m_hasFinished = value;
      emit finished();
    }

    /** \brief Method to be executed when a task is aborted
     *
     *  Overload this method when your task needs to do some proeccesing when it is aborted
     */
    virtual void onAbort() {}

  protected slots:
    void runWrapper();

  protected:
    void dispatcherPause();
    void dispatcherResume();
    bool isDispatcherPaused();
    void prepareToRun();

  private slots:
    void start();

  signals:
    void progress(int);
    void resumed();
    void paused();
    void finished();

  protected:
    SchedulerSPtr m_scheduler;

  private:
    int m_priority;

    bool m_pendingPause;
    bool m_pendingUserPause;
    bool m_isAborted;
    bool m_hasFinished;
    bool m_isPaused;
    bool m_isWaiting;

    QMutex         m_mutex;
    mutable QReadWriteLock m_descriptionLock;
    QWaitCondition m_pauseCondition;

    QString  m_description;
    Id m_id;
    bool m_isThreadAttached;
    bool m_hidden;

    friend class Scheduler;
  };
}

#endif // ESPINA_WORKER_H
