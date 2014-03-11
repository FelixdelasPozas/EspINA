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
void TaskQueue::orderedInsert(TaskSPtr worker)
{
  int w = 0;
  bool found = false;
  while (!found && w < size())
  {
    found = at(w)->id() > worker->id();
    if (!found)
      ++w;
  }
  insert(w, worker);
}

//-----------------------------------------------------------------------------
Scheduler::Scheduler(int period, QObject* parent)
: QObject(parent)
, m_period{period}
, m_lastId{0}
, m_maxNumRunningThreads{QThreadPool::globalInstance()->maxThreadCount() }
, m_abort{false}
{
  QThread *thread = new QThread();
  moveToThread(thread);
  connect(thread, SIGNAL(started()), this, SLOT(scheduleTasks()));
  thread->start();
}

//-----------------------------------------------------------------------------
Scheduler::~Scheduler()
{
  abortExecutingTasks();
}

//-----------------------------------------------------------------------------
void Scheduler::addTask(TaskSPtr task)
{
  m_mutex.lock();
  if (m_runningTasks[task->priority()].contains(task))
  {
    m_mutex.unlock();
    return;
  }

  task->setId(m_lastId++);
  m_runningTasks[task->priority()].orderedInsert(task);
  m_mutex.unlock();
  if (!task->isHidden())
    emit taskAdded(task);
}

//-----------------------------------------------------------------------------
void Scheduler::removeTask(TaskSPtr task)
{
  // NOTE: not used actually, used to be in Task destructor, but now task
  // are smartpointers, and deleting them from the list deletes them.
  m_mutex.lock();
  if (!m_runningTasks[task->priority()].contains(task))
  {
    m_mutex.unlock();
    return;
  }

  m_runningTasks[task->priority()].removeOne(task);
  m_mutex.unlock();
  if (!task->isHidden())
    emit taskRemoved(task);
}

//-----------------------------------------------------------------------------
void Scheduler::abortExecutingTasks()
{
  m_abort = true;
  m_mutex.lock();
  for (int priority = 4; priority >= 0; --priority)
  {
    for (TaskSPtr task : m_runningTasks[priority])
    {
      if (!task->hasFinished())
      {
        task->abort();
        if (!task->thread()->wait(1000))
          task->thread()->terminate();
      }
    }
    m_runningTasks[priority].clear();
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
void Scheduler::changePriority(TaskPtr task, int prevPriority)
{
  QMutexLocker lock(&m_mutex);

  for (auto otherTask : m_runningTasks[prevPriority])
    if (otherTask.get() == task)
    {
      m_runningTasks[prevPriority].removeOne(otherTask);
      m_runningTasks[task->priority()].orderedInsert(otherTask);
    }
}

//-----------------------------------------------------------------------------
void Scheduler::changePriority(TaskSPtr task, int prevPriority)
{
  changePriority(task.get(), prevPriority);
}

//-----------------------------------------------------------------------------
unsigned int Scheduler::numberOfTasks() const
{
  QMutexLocker lock(&m_mutex);

  unsigned int result;
  for(int priority = 4; priority >= 0; --priority)
    result += m_runningTasks[priority].size();

  return result;
}

//-----------------------------------------------------------------------------
void Scheduler::scheduleTasks()
{

  while (!m_abort)
  {
    QApplication::processEvents();

    m_mutex.lock();
    //std::cout << "Start Scheduling on thread " << thread() << std::endl;

    int numTask = 0;
    for (int priority = 4; priority >= 0; --priority)
    {
      int size = m_runningTasks[priority].size();
      numTask += size;
//      std::cout << "Priority " << priority << " has " << size << " tasks." << std::endl;
    }

    int num_running_threads = 0;

//    std::cout << "Scheduler has " << numTask << " tasks:" << std::endl;

    for (int priority = 4; priority >= 0; --priority)
    {
//       std::cout << "Updating Priority " << priority << std::endl;
      QList<TaskSPtr> deferredDeletionTaskList;

      for (TaskSPtr task : m_runningTasks[priority])
      {
        // TODO: this shouldn't be here?
        if (task == nullptr)
        {
          deferredDeletionTaskList << task;
          continue;
        }

        bool is_thread_attached = task->m_isThreadAttached;

        if (num_running_threads < m_maxNumRunningThreads
            && !(task->isPendingPause() || task->isWaiting() || task->isAborted() || task->hasFinished()))
        {
          if (is_thread_attached)
          {
            if (task->isDispatcherPaused())
            {
              task->dispatcherResume();
//               std::cout << "- " << task->id() << ": " << task->description().toStdString() << " resumed" << std::endl;
            }
            else
            {
//               std::cout << "- " << task->id() << ": " << task->description().toStdString() << " already running" << std::endl;
            }
          }
          else
          {
            task->start();
//             std::cout << "- " << task->id() << ": " << task->description().toStdString() << " started" << std::endl;
          }
          num_running_threads++;
        }
        else
        {
//           std::cout << "- " << task->id() << ": " << task->description().toStdString() << " is " << (!task->isRunning()?"not ":"") << "running" << std::endl;
          bool hasBeenAbortedWithoutRunning = task->isAborted() && !is_thread_attached;
          if (task->hasFinished() || hasBeenAbortedWithoutRunning)
          {
//             { // DEBUG
//               if (task->hasFinished())
//               {
//                 std::cout << "- " << task->id() << ": " << task->description().toStdString() << " has finished" << (task->isAborted()?" and was aborted":"") << std::endl;
//               }
//               else
//               {
//                 std::cout << "- " << task->id() << ": " << task->description().toStdString() << " was aborted without running" << std::endl;
//               }
//             }
            deferredDeletionTaskList << task;

            if (!task->isHidden())
            {
              emit taskRemoved(task);
            }
          }
          else
          {
            // Waiting tasks also fulfill these conditions so they must be paused by dispatcher
            if (!task->isPendingPause() && is_thread_attached && !task->isDispatcherPaused())
            {
              task->dispatcherPause();
//             std::cout << "- " << task->id() << ": " << task->description().toStdString() << " was paused by scheduler" << std::endl;
            }
            else
              if (task->isAborted() && task->isDispatcherPaused())
              {
                task->dispatcherResume();
              }
          }
        }

//         { // DEBUG
//           if (task->isPaused())
//           {
//             std::cout << "- " << task->id() << ": " << task->description().toStdString() << " was paused by the user" << std::endl;
//           }
//           else if (task->isAborted())
//           {
//             std::cout << "- " << task->id() << ": " << task->description().toStdString() << " was aborted but hasn't finished yet" << std::endl;
//           }
//           else
//           {
//             std::cout << "- " << task->id() << ": " << task->description().toStdString() << " is ready to start" << std::endl;
//           }
//         }
      }
      for (auto task : deferredDeletionTaskList)
        m_runningTasks[priority].removeOne(task);

//      for (auto task : m_runningTasks[priority])
//      {
//        std::cout << task->id() << " - ";
//        std::cout << (task->isPaused() ? "paused " : "");
//        std::cout << (task->isPendingPause() ? "paused " : "");
//        std::cout << (task->isAborted() ? "aborted " : "");
//        std::cout << (task->isDispatcherPaused() ? " dispacherPaused " : "");
//        std::cout << (task->hasFinished() ? "finished " : "");
//        std::cout << (task->isRunning() ? "running " : "");
//        std::cout << (task->isHidden() ? "hidden " : "");
//        std::cout << std::endl;
//      }
    }
    m_mutex.unlock();

    usleep(m_period);
  }
}
