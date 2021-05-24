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
#include "Scheduler.h"
#include "Task.h"

// C++
#include <iostream>
#include <unistd.h>
#include <chrono>

// Qt
#include <QThread>
#include <QApplication>
#include <QDebug>

using namespace ESPINA;
using namespace std::chrono;

//-----------------------------------------------------------------------------
void printTask(TaskSPtr task, const QString& message)
{
  std::cout << "- " << task->id() << ": " << task->description().toStdString() << " " << message.toStdString() << std::endl;
}

//-----------------------------------------------------------------------------
void Scheduler::TaskQueue::orderedInsert(TaskSPtr worker)
{
  int w = 0;
  bool found = false;

  while (!found && w < size())
  {
    found = at(w)->id() > worker->id();

    if (!found) ++w;
  }

  insert(w, worker);
}

//-----------------------------------------------------------------------------
Scheduler::Scheduler(int period, QObject* parent)
: QObject             {parent}
, m_period            {period}
, m_lastId            {0}
, m_maxNumRunningTasks{maxRunningTasks()}
, m_abort             {false}
{
  auto thread = new QThread();

  moveToThread(thread);

  connect(thread, SIGNAL(started()),
          this,   SLOT(scheduleTasks()));

  connect(this,   SIGNAL(destroyed(QObject *)),
          thread, SLOT(quit()));

  connect(thread, SIGNAL(finished()),
          thread, SLOT(deleteLater()));


  // TODO: better task scheduling when a task gets blocked, until then just set the bar high
  //       enough to avoid starvation. Need to know when a task is really running or waiting
  //       maybe by using a counter in canExecute() ? Cheap but dirty.
  // NOTE: can be "task->thread()->yieldCurrentThread();" a solution with the cycle counter?
  m_maxNumRunningTasks = MAX_TASKS;

  thread->start();
}

//-----------------------------------------------------------------------------
Scheduler::~Scheduler()
{
  abort();
}

//-----------------------------------------------------------------------------
void Scheduler::addTask(TaskSPtr task)
{
  if(m_abort) return;

  QMutexLocker lock(&m_insertionMutex);

  task->setId(m_lastId++);

  m_insertionBuffer << task;

  if (!task->isHidden())
  {
    emit taskAdded(task);
  }
}

//-----------------------------------------------------------------------------
void Scheduler::abort()
{
  m_abort = true;

  // wait for the scheduler thread to end scheduleTask() method. Period is microsec so its converted to millisec.
  m_waitMutex.lock();
  m_wait.wait(&m_waitMutex, 3 * m_period/1000);
  m_waitMutex.unlock();

  {
    QMutexLocker priorityLock(&m_priorityMutex);
    m_priorityBuffer.clear();
  }

  {
    QMutexLocker insertionLock(&m_insertionMutex);
    m_insertionBuffer.clear();
  }

  QMutexLocker lock(&m_mutex);

  for (auto priority: {Priority::VERY_HIGH, Priority::HIGH, Priority::NORMAL, Priority::LOW, Priority::VERY_LOW})
  {
    for (auto task : m_scheduledTasks[priority])
    {
      QMutexLocker sbLock(&task->m_submissionMutex);
      task->m_submitted = false;

      if (!task->hasFinished())
      {
        task->abort();

        if (!task->thread()->wait(1000)) task->thread()->terminate();
      }
    }
    m_scheduledTasks[priority].clear();
  }
  m_scheduledTasks.clear();
}

//-----------------------------------------------------------------------------
void Scheduler::changePriority(TaskPtr task, Priority prevPriority)
{
  QMutexLocker lock(&m_priorityMutex);
  if (!m_priorityBuffer.contains(task))
  {
    m_priorityBuffer[task] = prevPriority;
  }
}

//-----------------------------------------------------------------------------
void Scheduler::changePriority(TaskSPtr task, Priority prevPriority)
{
  changePriority(task.get(), prevPriority);
}

//-----------------------------------------------------------------------------
unsigned int Scheduler::maxRunningTasks()
{
  /* NOTE: should be "QThreadPool::globalInstance()->maxThreadCount();" or "std::thread::hardware_concurrency();" 
   * but produces starvation if a file has more counting frames.
   */
  return MAX_TASKS;
}

//-----------------------------------------------------------------------------
void Scheduler::scheduleTasks()
{
  disconnect(this->thread(), SIGNAL(started()),
             this,           SLOT(scheduleTasks()));

  high_resolution_clock::time_point start;
  high_resolution_clock::time_point last;
  high_resolution_clock::time_point end;

  while (!m_abort)
  {
    QApplication::processEvents();

    proccessTaskInsertion();

//     std::cout << "Start Scheduling" << std::endl;

    reschedule();

    {
      QMutexLocker lock(&m_mutex);

      start = high_resolution_clock::now();

  //     std::cout << "\t Scheduler thread " << thread() << std::endl;
  //     int numTasks = 0;
  //     for (auto priority: {Priority::VERY_HIGH, Priority::HIGH, Priority::NORMAL, Priority::LOW, Priority::VERY_LOW})
  //     {
  //       int size = m_scheduledTasks[priority].size();
  //       numTasks += size;
  //       std::cout << "Priority " << (int)priority << " has " << size << " tasks." << std::endl;
  //     }
  //     std::cout << "Scheduler has " << numTasks << " tasks:" << std::endl;

      unsigned int num_running_threads = 0;

      for (auto priority: {Priority::VERY_HIGH, Priority::HIGH, Priority::NORMAL, Priority::LOW, Priority::VERY_LOW})
      {
  //       std::cout << "Updating Priority " << priority << std::endl;
        QList<TaskSPtr> deferredDeletionTaskList;

        for(auto task: m_scheduledTasks[priority])
        {
//        for (auto scheduledTask : m_scheduledTasks[priority])
//        {
//          auto task = scheduledTask.Task;
          const auto isEdgesTask = (priority == Priority::VERY_HIGH) && task->description().contains("Edges", Qt::CaseInsensitive);
          const auto is_thread_attached = task->isExecutingOnThread();

          if (((num_running_threads < m_maxNumRunningTasks) || isEdgesTask) && canExecute(task))
          {
            //printTask(task, "should be running");
            if (is_thread_attached)
            {
              if (task->isDispatcherPaused())
              {
                task->dispatcherResume();
                //printTask(task, "resumed by scheduler");
              }
              else
              {
  //               printTask(task, "already running");
              }
            }
            else
            {
              task->startThreadExecution();
              //printTask(task, "started");
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
            }
            else
            {
              // Waiting tasks also fulfill these conditions so they must be paused by dispatcher
              if (!task->isPendingPause() && is_thread_attached && !task->isDispatcherPaused())
              {
                task->dispatcherPause();
                //printTask(task, " was paused by Scheduler");
              }
              else if (task->isAborted() && task->isDispatcherPaused())
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
        {
          removeTask(priority, task);
        }

  //      for (auto task : m_runningTasks[priority])
  //      {
  //         printState(task);
  //      }
      }
    }

    end  = high_resolution_clock::now();

    int lastSchedulingTime = duration_cast<microseconds>(end - start).count();

//     std::cout << "Inter Schedule time" << duration_cast<microseconds>(start - last).count() << std::endl;
//     std::cout << "Scheduling End after " << lastSchedulingTime << std::endl;
//
//     last = start;

    usleep(std::max(0, m_period - lastSchedulingTime));
  }

  m_wait.wakeAll();
}

//-----------------------------------------------------------------------------
void Scheduler::proccessTaskInsertion()
{
  if(m_abort) return;

  QMutexLocker insertionLock(&m_insertionMutex);
  QMutexLocker priorityLock(&m_priorityMutex);

  for (auto task : m_insertionBuffer)
  {
    if(task->description().contains("Edges:"))
    {
      m_scheduledTasks[Priority::VERY_HIGH].orderedInsert(task);
    }
    else
    {
      m_scheduledTasks[task->priority()].orderedInsert(task);
    }
    m_priorityBuffer.remove(task.get());
  }

  m_insertionBuffer.clear();
}

//-----------------------------------------------------------------------------
void Scheduler::reschedule()
{
  proccessPriorityChanges();

  //roundRobinShift();
}

//-----------------------------------------------------------------------------
void Scheduler::proccessPriorityChanges()
{
  QMutexLocker priorityLock(&m_priorityMutex);

  for (auto task : m_priorityBuffer.keys())
  {
    auto prevPriority = m_priorityBuffer[task];

    for (auto prevTask : m_scheduledTasks[prevPriority])
    {
//    for (auto scheduledTask : m_scheduledTasks[prevPriority])
//    {
//      auto prevTask = scheduledTask.Task;
      if (prevTask.get() == task)
      {
//        m_scheduledTasks[prevPriority].removeOne(scheduledTask);
        m_scheduledTasks[prevPriority].removeOne(prevTask);
        m_scheduledTasks[task->priority()].orderedInsert(prevTask);
        break;
      }
    }
  }

  m_priorityBuffer.clear();
}

//-----------------------------------------------------------------------------
void Scheduler::roundRobinShift()
{
  return;
//  for (auto priority: {Priority::VERY_HIGH, Priority::HIGH, Priority::NORMAL, Priority::LOW, Priority::VERY_LOW})
//  {
//    QList<ScheduledTask> rescheduleList;
//
//    auto &taskQueue = m_scheduledTasks[priority];
//
//    for (auto &scheduledTask : taskQueue)
//    {
//      auto task = scheduledTask.Task;
//      if (!task->isDispatcherPaused() && task->isRunning() && !scheduledTask.consumeCicle())
//      {
//         printTask(task, "used all its cicles");
//        scheduledTask.restoreCicles();
//        rescheduleList << scheduledTask;
//      }
//       else
//       {
//         printTask(task, QString("has %1 remaining cicles").arg(scheduledTask.Cicles));
//       }
//    }
//
//    for (auto &scheduledTask : rescheduleList)
//    {
//      taskQueue.removeOne(scheduledTask);
//      taskQueue << scheduledTask;
//    }
//  }
}

//-----------------------------------------------------------------------------
void Scheduler::removeTask(Priority priority, TaskSPtr task)
{
  QMutexLocker sumbissionLock(&task->m_submissionMutex);
  if (task->m_submitted)
  {
    m_scheduledTasks[priority].removeOne(task);
    m_insertionBuffer.removeOne(task);

    task->m_submitted = false;

    if (!task->isHidden())
    {
      emit taskRemoved(task);
    }
  }
}

//-----------------------------------------------------------------------------
unsigned int Scheduler::numberOfTasks() const
{
  QMutexLocker lock(&m_mutex);

  unsigned int result = 0;
  auto priorities = {Priority::VERY_HIGH, Priority::HIGH, Priority::NORMAL, Priority::LOW, Priority::VERY_LOW};
  std::for_each(priorities.begin(), priorities.end(), [&result, this] (const Priority priority) { result += m_scheduledTasks[priority].size(); });

  result += m_insertionBuffer.size();

  return result;
}

//-----------------------------------------------------------------------------
bool Scheduler::canExecute(TaskSPtr task) const
{
  return !(task->isPendingPause() || task->isAborted() || task->hasFinished());
}

//-----------------------------------------------------------------------------
void Scheduler::printState(TaskSPtr task) const
{
  std::cout << task->id() << " Priority[" << (int)task->priority() << "]" << " - " << task->description().toStdString() << " - ";
  std::cout << (task->isExecutingOnThread() ? "Executing on thread " : "");
  std::cout << (task->isPaused() ? "paused " : "");
  std::cout << (task->isPendingPause() ? "pending pause " : "");
  std::cout << (task->isAborted() ? "aborted " : "");
  std::cout << (task->isDispatcherPaused() ? " dispacherPaused " : "");
  std::cout << (task->needsRestart() ? "needsRestart " : "");
  std::cout << (task->hasFinished() ? "finished " : "");
  std::cout << (task->isRunning() ? "running " : "");
  std::cout << (task->isHidden() ? "hidden " : "");
  std::cout << std::endl;
}
