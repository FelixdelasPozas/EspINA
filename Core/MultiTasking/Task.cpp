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
#include "Task.h"

#include "Scheduler.h"

#include <iostream>
#include <unistd.h>

#include <QCoreApplication>
#include <QThread>

using namespace EspINA;

//-----------------------------------------------------------------------------
Task::Task(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
, m_priority{Priority::NORMAL}
, m_pendingPause{false}
, m_pendingUserPause{false}
, m_isAborted{false}
, m_hasFinished{false}
, m_isPaused{false}
, m_isWaiting{false}
, m_id {0}
, m_isThreadAttached{false}
, m_hidden{false}
{
  prepareToRun();

  if (m_scheduler != nullptr)
  {
    moveToThread(m_scheduler->thread());
  }
}

//-----------------------------------------------------------------------------
Task::~Task()
{
  //std::cout << m_id << ": Destroying " << m_description.toStdString() << " in " << (m_isThreadAttached?"attached":"") << " thread " << QThread::currentThread() << std::endl;
  QMutexLocker lock(&m_mutex);
  if (m_isThreadAttached)
    thread()->quit();
}

//-----------------------------------------------------------------------------
void Task::setPriority(const int value)
{
  int previous = m_priority;

  if (previous != value)
  {
    m_priority = value;

    if (m_scheduler != nullptr)
      m_scheduler->changePriority(this, previous);
  }
}

//-----------------------------------------------------------------------------
void Task::submit(TaskSPtr task)
{
  task->prepareToRun();

  if (task->m_scheduler != nullptr)
  {
    task->m_scheduler->addTask(task);
  }
  else
  {
    task->runWrapper();
  }
}

//-----------------------------------------------------------------------------
void Task::pause()
{
  m_mutex.lock();
//  std::cout << m_description.toStdString() << " has been paused by the user" << std::endl;
  m_pendingUserPause = true;
  m_mutex.unlock();

  dispatcherPause();
}

//-----------------------------------------------------------------------------
void Task::resume()
{
  QMutexLocker lock(&m_mutex);
//  std::cout << m_description.toStdString() << " has been resumed by the user" << std::endl;
  m_pendingUserPause = false;
}

//-----------------------------------------------------------------------------
bool Task::isPendingPause() const
{
  return m_pendingUserPause;
}

//-----------------------------------------------------------------------------
void Task::abort()
{
  QMutexLocker lock(&m_mutex);
//  std::cout << m_description.toStdString() << " has been cancelled" << std::endl;
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

    m_isPaused = true;
    m_pauseCondition.wait(&m_mutex);
    m_isPaused = false;
    m_pendingPause = false;

    if (notify)
      emit resumed();
  }

  return !m_isAborted;
}

//-----------------------------------------------------------------------------
void Task::runWrapper()
{
  run();

  m_pendingPause = false;
  m_pendingUserPause = false;

  setFinished(true);
}

//-----------------------------------------------------------------------------
void Task::dispatcherPause()
{
  QMutexLocker lock(&m_mutex);
  //std::cout << m_description.toStdString() << " has been paused" << std::endl;
  m_pendingPause = true;
}

//-----------------------------------------------------------------------------
void Task::dispatcherResume()
{
  if (!m_pendingUserPause)
  {
    //std::cout << m_description.toStdString() << " has been resumed" << std::endl;
    m_pauseCondition.wakeAll();
  }
}

//-----------------------------------------------------------------------------
bool Task::isDispatcherPaused()
{
  return m_pendingPause;
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
  m_pendingPause = false;
  m_pendingUserPause =false;
  m_isAborted = false;
  m_hasFinished = false;
  m_isPaused = false;
  m_isWaiting = false;
  m_isThreadAttached = false;
}

//-----------------------------------------------------------------------------
void Task::start()
{
  QMutexLocker lock(&m_mutex);

  //std::cout << "Starting " << description().toStdString() << " inside thread " << thread() << std::endl;

  if (!m_isThreadAttached)
  {
    TestThread *thread = new TestThread();

    m_isThreadAttached = true;

    moveToThread(thread);

    connect(thread, SIGNAL(started()), this, SLOT(runWrapper()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
  }
}