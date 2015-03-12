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
#include <unistd.h>

#include <QCoreApplication>
#include <QThread>
 
using namespace ESPINA;
using namespace std;

int scheduler_simple_task_restart( int argc, char** argv )
{
  int error = 0;

  int period = 1000;// 1 ms

  int sleepTime = period;
  int taskTime  = SleepyTask::Iterations*sleepTime;

  int firstExecTime   = 6*period;
  int restartExecTime = firstExecTime / 2;

  QCoreApplication app(argc, argv);
  
  auto scheduler  = make_shared<Scheduler>(period);
  auto sleepyTask = make_shared<SleepyTask>(sleepTime, scheduler);
  sleepyTask->setDescription("Simple Task");
  
  Task::submit(sleepyTask);
  
  usleep(firstExecTime);

  if (!sleepyTask->isExecuting()) {
    error = 1;
    std::cerr << "Unexpected paused sleepy task" << std::endl;
  }

  auto lastResult = sleepyTask->Result;

  sleepyTask->restart();

  usleep(restartExecTime);

  auto currentResult = sleepyTask->Result;
  if (lastResult < currentResult)
  {
    error = 1;
    std::cerr << "Unexpected restarted value: " << currentResult << " != " << lastResult << std::endl;
  }

  usleep(1.5*taskTime);

  if (!sleepyTask->isFinished())
  {
    error = 1;
    std::cerr << "Task has not finished executing " << std::endl;
  }

  Task::submit(sleepyTask);

  usleep(2*period);

  if (!sleepyTask->isExecuting()) {
    error = 1;
    std::cerr << "Unexpected paused task" << std::endl;
  }

  lastResult = sleepyTask->Result;

  Task::submit(sleepyTask);

  usleep(period);

  if (lastResult < sleepyTask->Result)
  {
    error = 1;
    std::cerr << "Unexpected restarted value" << std::endl;
  }

  usleep(1.5*taskTime);

  return error;
}