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

#include "Scheduler.h"

#include "Task.h"

#include <iostream>

#include <QThread>
#include <QThreadPool>
#include <QApplication>
#include <unistd.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
void TaskQueue::orderedInsert(Task* worker)
{
  int  w = 0;
  bool found = false;
  while (!found && w < size()) {
    found = at(w)->id() > worker->id();
    if (!found) ++w;
  }
  insert(w, worker);
}

//-----------------------------------------------------------------------------
Scheduler::Scheduler(int period, QObject* parent)
: QObject(parent)
, m_period{period}
, m_lastId{0}
, m_maxNumRunningThreads{QThreadPool::globalInstance()->maxThreadCount()}
, m_abort{false}
{
  QThread *thread = new QThread();
  moveToThread(thread);
  connect(thread, SIGNAL(started()),
          this, SLOT(scheduleTasks()));
  thread->start();
}

//-----------------------------------------------------------------------------
Scheduler::~Scheduler()
{
  abortExecutingTasks();
}

//-----------------------------------------------------------------------------
void Scheduler::addTask(Task* task) {
  task->setId(m_lastId++);

  QMutexLocker lock(&m_mutex);
  m_runningTasks[task->priority()].orderedInsert(task);

  emit taskAdded(task);
}

//-----------------------------------------------------------------------------
void Scheduler::removeTask(Task* task)
{
  QMutexLocker lock(&m_mutex);
  m_runningTasks[task->priority()].removeOne(task);

  emit taskRemoved(task);
}

//-----------------------------------------------------------------------------
void Scheduler::abortExecutingTasks()
{
  m_abort = true;
}


//-----------------------------------------------------------------------------
void Scheduler::changePriority(Task* task, int prevPriority )
{
  QMutexLocker lock(&m_mutex);

  //std::cout << "Changing priority of " << task->description().toStdString() << std::endl;
  m_runningTasks[prevPriority].removeOne(task);
  m_runningTasks[task->priority()].orderedInsert(task);
}

//-----------------------------------------------------------------------------
void Scheduler::scheduleTasks() {

  while(!m_abort){
    QApplication::processEvents();

    m_mutex.lock();
    //std::cout << "Start Scheduling on thread " << thread() << std::endl;

    int numTask = 0;
    for (int priority = 4; priority >= 0; --priority) {
      numTask += m_runningTasks[priority].size();
    }

    int num_running_threads = 0;

//     std::cout << "Scheduler has "<< numTask << " tasks:" << std::endl;

    for (int priority = 4; priority >= 0; --priority) {
//       std::cout << "Updating Priority " << priority << std::endl;

      for(Task *task : m_runningTasks[priority])
      {
        bool is_thread_attached = task->m_isThreadAttached;

        if (num_running_threads < m_maxNumRunningThreads && !(task->isPaused() || task->isAborted() || task->hasFinished() )) {
          if (is_thread_attached) {
            if (task->isDispatcherPaused()) {
              task->dispatcherResume();
              // std::cout << "- " << worker->description().toStdString() << " resumed" << std::endl;
            } else {
              // std::cout << "- " << worker->description().toStdString() << " already running" << std::endl;
            }
          } else {
            task->start();
            // std::cout << "- " << worker->description().toStdString() << " started" << std::endl;
          }
          num_running_threads++;
        } else {
          if (task->isPaused()) {
            // std::cout << "- " << worker->description().toStdString() << " was paused by the user" << std::endl;
          } else if (task->isAborted() || task->hasFinished()) {
            // std::cout << "- " << worker->description().toStdString() << " was aborted" << std::endl;
            // std::cout << "- " << worker->description().toStdString() << " has finished" << std::endl;
            m_runningTasks[task->priority()].removeOne(task);
            emit taskRemoved(task);
          } else if (is_thread_attached) {
            task->dispatcherPause();
            // std::cout << "- " << worker->description().toStdString() << " was paused by scheduler" << std::endl;
          } else {
            // std::cout << "- " << worker->description().toStdString() << " is ready to start" << std::endl;
          }
        }
      }
    }
    m_mutex.unlock();

    usleep(m_period);
  }
}
