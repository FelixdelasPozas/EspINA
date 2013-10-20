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

#include <Scheduler.h>

#include "SleepyTask.h"

#include <iostream>
#include <memory>
#include <unistd.h>

#include <QThreadPool>
 
using namespace EspINA;
using namespace std;

int scheduler_change_task_priority( int argc, char** argv )
{
  int error = 0;
  
  int schedulerPeriod = 5000;
  int taskSleepTime   = 4*schedulerPeriod;
  int taskTime        = 10*taskSleepTime;
  
  SchedulerSPtr scheduler = SchedulerSPtr(new Scheduler(schedulerPeriod));
  
  int numThreads = QThreadPool::globalInstance()->maxThreadCount();
  int numTasks   = numThreads + 5;
  
  std::vector<unique_ptr<SleepyTask>> tasks;
  
  for (int i = 0; i < numTasks; ++i) {
    tasks.push_back(unique_ptr<SleepyTask>(new SleepyTask(taskSleepTime, scheduler)));
    tasks.at(i)->setDescription(QString("Task %1").arg(i));
    tasks.at(i)->submit();
  }
  
  usleep(schedulerPeriod);
  
  for (int i = 0; i < numThreads; ++i) {
    if (tasks.at(i)->Result != 0) {
      error = 1;      
      std::cerr << "Task " << i << " should be running" << std::endl;
    }
  }
  
  if (tasks.at(numTasks-1)->Result != -1) {
    error = 1;      
    std::cerr << "Last Task should be paused by the dispatcher" << std::endl;
  }
  
  tasks.at(numTasks-1)->setPriority(Priority::VERY_HIGHT);
  
  usleep(schedulerPeriod);

  for (int i = 0; i < numThreads - 1; ++i) {
    if (tasks.at(i)->Result != 0) {
      error = 1;      
      std::cerr << "Task " << i << " should be running" << std::endl;
    }
  }
  
  if (tasks.at(numTasks-1)->Result != 0) {
    error = 1;      
    std::cerr << "Last Task should be paused by the dispatcher" << std::endl;
  }
  
  usleep((numTasks + 1) * taskTime);
  
  for (int i = 0; i < numTasks; ++i) {
    if (tasks.at(i)->Result != 1) {
      error = 1;      
      std::cerr << "Task " << i << " should have finishd" << std::endl;
    }
  }
  
  return error;
}