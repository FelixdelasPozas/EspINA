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

// ESPINA
#include "Task.h"
#include "Scheduler.h"

// C++
#include <iostream>
#include <unistd.h>

// Qt
#include <QThread>
#include <QDebug>
#include <QCoreApplication>

using namespace ESPINA;

//-----------------------------------------------------------------------------
Task::Task(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
, m_thread         {nullptr}
, m_executingThread{nullptr}
, m_submitted      {false}
, m_priority{Priority::NORMAL}
, m_isRunning       {false}
, m_pendingPause    {false}
, m_pendingUserPause{false}
, m_isAborted       {false}
, m_hasFinished     {false}
, m_isPaused        {false}
, m_isWaiting       {false}
, m_needsRestart    {false}
, m_id              {0}
, m_hidden          {false}
{
  prepareToRun();

  if (m_scheduler != nullptr)
  {
    moveToThread(m_scheduler->thread());
  }

  connect(this, SIGNAL(finished()),
          this, SLOT(onTaskFinished()));
}

//-----------------------------------------------------------------------------
Task::~Task()
{
  //std::cout << m_id << ": Destroying " << m_description.toStdString() << " in " << (m_isThreadAttached?"attached":"") << " thread " << QThread::currentThread() << std::endl;
  QMutexLocker lock(&m_mutex);

  if (m_executingThread)
  {
    qWarning() << "Someone did it!";
    m_executingThread->deleteLater();
  }
}

//-----------------------------------------------------------------------------
void Task::setPriority(Priority value)
{
  Priority previous = m_priority;

  if (previous != value)
  {
    m_priority = value;

    QMutexLocker lock(&m_submissionMutex);
    if (m_submitted && m_scheduler)
    {
      m_scheduler->changePriority(this, previous);
    }
  }
}

//-----------------------------------------------------------------------------
void Task::pause()
{
  m_pendingUserPause = true;

  dispatcherPause();
}

//-----------------------------------------------------------------------------
void Task::resume()
{
  m_pendingUserPause = false;
}

//-----------------------------------------------------------------------------
bool Task::isPendingPause() const
{
  return m_pendingUserPause;
}

//-----------------------------------------------------------------------------
void Task::restart()
{
  m_needsRestart = true;
}

//-----------------------------------------------------------------------------
bool Task::isRunning() const
{
  return m_isRunning && !isPendingPause();
}

//-----------------------------------------------------------------------------
bool Task::isAborted() const
{
  return m_isAborted;
}

//-----------------------------------------------------------------------------
bool Task::hasFinished() const
{
  return m_hasFinished && !m_needsRestart;
}

//-----------------------------------------------------------------------------
void Task::submit(TaskSPtr task)
{

  if (task->m_scheduler != nullptr)
  {
    QMutexLocker lock(&task->m_submissionMutex);

    if (!task->m_submitted)
    {
      task->prepareToRun();
      task->m_scheduler->addTask(task);
      task->m_submitted = true;
    }
    else
    {
      // Submitting an already submitted task implies a restart
      task->restart();
    }
  }
  else
  {
    task->runWrapper();
  }
}

//-----------------------------------------------------------------------------
void Task::abort()
{
  m_isAborted = true;

  onAbort();
}

//-----------------------------------------------------------------------------
bool Task::canExecute()
{
  // NOTE: Necessary to receive signals from other threads
  QCoreApplication::processEvents();

  QMutexLocker lock(&m_mutex);
  if (m_pendingPause)
  {
    bool notify = m_pendingUserPause;

    if (notify)
      emit paused();

    m_isRunning    = false;
    m_isPaused     = true;
    m_pauseCondition.wait(&m_mutex);
    m_isRunning    = true;
    m_isPaused     = false;
    m_pendingPause = false;

    if (notify)
      emit resumed();
  }

  return !(m_isAborted || m_needsRestart);
}

//-----------------------------------------------------------------------------
void Task::setFinished(bool value)
{
  m_hasFinished = value;

  emit finished();

  QCoreApplication::sendPostedEvents();
}

//-----------------------------------------------------------------------------
void Task::runWrapper()
{
  m_isRunning    = true;
  m_needsRestart = true;

  while (m_needsRestart)
  {
    m_needsRestart = false;

    if(!isAborted())
    {
      run();

      canExecute();
    }
  }

  m_isRunning        = false;
  m_pendingPause     = false;
  m_pendingUserPause = false;

  setFinished(true);
}

//-----------------------------------------------------------------------------
void Task::dispatcherPause()
{
//   qDebug() << m_id << " paused by scheduler";
  m_pendingPause = true;
}

//-----------------------------------------------------------------------------
void Task::dispatcherResume()
{
  if (!m_pendingUserPause)
  {
//     qDebug() << m_id << " resumed by scheduler";
    m_pauseCondition.wakeAll();
    m_pendingPause = false;
  }
}

//-----------------------------------------------------------------------------
bool Task::isDispatcherPaused()
{
  return m_pendingPause;
}

//-----------------------------------------------------------------------------
bool Task::needsRestart() const
{
  return m_needsRestart;
}


class TestThread: public QThread
{
  public:
    virtual ~TestThread()
    {
//      std::cout << "Destroying thread" << std::endl;
    }
};

//-----------------------------------------------------------------------------
void Task::prepareToRun()
{
  m_isRunning        = false;
  m_pendingPause     = false;
  m_pendingUserPause = false;
  m_isAborted        = false;
  m_hasFinished      = false;
  m_isPaused         = false;
  m_isWaiting        = false;
}

//-----------------------------------------------------------------------------
void Task::startThreadExecution()
{
  QMutexLocker lock(&m_mutex);

  Q_ASSERT(!m_executingThread);

  m_thread          = thread();
  m_executingThread = new TestThread();

  moveToThread(m_executingThread);

  connect(m_executingThread, SIGNAL(started()),
          this,              SLOT(runWrapper()));
  connect(m_executingThread, SIGNAL(finished()),
          m_executingThread, SLOT(deleteLater()));

  m_executingThread->start();
}

//-----------------------------------------------------------------------------
void Task::onTaskFinished()
{
  if (m_needsRestart)
  {
    runWrapper();
  }
  else if (isExecutingOnThread())
  {
    finishThreadExecution();
  }
}

//-----------------------------------------------------------------------------
void Task::finishThreadExecution()
{
  QMutexLocker lock(&m_mutex);

//   std::cout << description().toStdString() << " finishing executing thread " << m_executingThread << std::endl;
  Q_ASSERT(m_executingThread);

  moveToThread(m_thread);

//   disconnect(this,     SIGNAL(finished()),
//              this,     SLOT(finishThreadExecution()));

  m_executingThread->quit();

  m_executingThread = nullptr;
}

//-----------------------------------------------------------------------------
bool Task::isExecutingOnThread() const
{
  return m_executingThread != nullptr;
}