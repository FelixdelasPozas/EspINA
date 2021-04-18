/*
 *    Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

    reportProgress(i*10);
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
