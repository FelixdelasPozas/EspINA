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
, m_hasFinished{false}
, m_priority{Priority::NORMAL}
, m_pendingAbort{false}
, m_pendingPause{false}
, m_pendingUserPause{false}
, m_aborted{false}
, m_id{0}
, m_isThreadAttached{false}
, m_hidden{false}
{
  if (m_scheduler != nullptr)
  {
    moveToThread(m_scheduler->thread());
  }
}

//-----------------------------------------------------------------------------
Task::~Task()
{
  // std::cout << "Destroying " << m_description.toStdString() << " in " << (m_isThreadAttached?"attached":"") << " thread " << QThread::currentThread() << std::endl;
  m_mutex.lock();
  if (m_isThreadAttached) thread()->quit();
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
void Task::setPriority(const int value)
{
  int previous = m_priority;

  if (previous != value) {
    m_priority = value;

    if (m_scheduler != nullptr)
    {
      m_scheduler->changePriority(this, previous);
    }
  }
}

//-----------------------------------------------------------------------------
void Task::submit(TaskSPtr task) {
  if (task->m_scheduler != nullptr)
  {
    task->m_scheduler->addTask(task);
  } else {
    task->runWrapper();
  }
}

//-----------------------------------------------------------------------------
void Task::pause() {
  m_mutex.lock();
//  std::cout << m_description.toStdString() << " has been paused by the user" << std::endl;
  m_pendingUserPause = true;
  m_mutex.unlock();

  dispatcherPause();
  //m_dispatcher->scheduleTasks();
}

//-----------------------------------------------------------------------------
void Task::resume() {
  m_mutex.lock();
//  std::cout << m_description.toStdString() << " has been resumed by the user" << std::endl;
  m_pendingUserPause = false;
  m_mutex.unlock();

  //m_dispatcher->scheduleTasks();
}

//-----------------------------------------------------------------------------
bool Task::isPaused() const
{
  return m_pendingUserPause;
}

//-----------------------------------------------------------------------------
void Task::abort() {
  m_mutex.lock();
//  std::cout << m_description.toStdString() << " has been cancelled" << std::endl;
  m_pendingAbort = true;
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
bool Task::canExecute() {
  // NOTE: Necessary to receive signals from other threads
  QCoreApplication::processEvents();

  m_mutex.lock();
  if (m_pendingPause) {
    bool notify = m_pendingUserPause;

    if (notify) emit paused();

    m_paused.wait(&m_mutex);
    m_pendingPause = false;

    if (notify) emit resumed();
  }
  m_mutex.unlock();

  return !m_pendingAbort;
}

//-----------------------------------------------------------------------------
void Task::runWrapper()
{
  run();

  m_hasFinished  = !m_pendingAbort;
  m_aborted      = m_pendingAbort;
  m_pendingAbort = false;

  emit finished();
}

//-----------------------------------------------------------------------------
void Task::dispatcherPause()
{
  m_mutex.lock();
  //std::cout << m_description.toStdString() << " has been paused" << std::endl;
  m_pendingPause = true;
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
void Task::dispatcherResume()
{
  m_mutex.lock();
  if (!m_pendingUserPause) {
    //std::cout << m_description.toStdString() << " has been resumed" << std::endl;
    m_paused.wakeAll();
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
bool Task::isDispatcherPaused()
{
  return m_pendingPause;
}

class TestThread 
: public QThread {
public:
    virtual ~TestThread() {
//      std::cout << "Destroying thread" << std::endl;
    }
};

//-----------------------------------------------------------------------------
void Task::start()
{
  QMutexLocker lock(&m_mutex);

  //std::cout << "Starting " << description().toStdString() << " inside thread " << thread() << std::endl;

  if (!m_isThreadAttached) {
    TestThread *thread = new TestThread();

    m_isThreadAttached = true;

    moveToThread(thread);

    connect(thread, SIGNAL(started()),  this,   SLOT(runWrapper()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
  }
}
