/*
 * Copyright 2013 Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "SleepyTask.h"

#include <iostream>
#include <unistd.h>

using namespace ESPINA;

SleepyTask::SleepyTask(int sleepTime, SchedulerSPtr scheduler)
: Task{scheduler}
, Result{-1}
, m_sleepTime{sleepTime}
{
}

SleepyTask::~SleepyTask()
{
}


void SleepyTask::run()
{
  int  i = 0;
  Result = 0;
  while (canExecute() && i < Iterations)
  {
    std::cout << description().toStdString() << " is working " << i+1 << "/10 on thread " << thread() << std::endl;
    usleep(m_sleepTime);
    i++;

    Result = i;

    emit progress(i*10);
  }
  std::cout << description().toStdString() << " ended" << std::endl;
}


bool SleepyTask::isFinished()
{
  return Result == Iterations;
}

bool SleepyTask::isExecuting()
{
  return Result > -1 && Result < Iterations;
}
