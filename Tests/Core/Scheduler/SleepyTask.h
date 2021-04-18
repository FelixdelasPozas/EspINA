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

#ifndef ESPINA_SLEEPYWORKER_H
#define ESPINA_SLEEPYWORKER_H

#include <Task.h>

namespace ESPINA {

  class SleepyTask 
  : public Task
  {
  public:
    static const int Iterations = 10;

  public:
    explicit SleepyTask(int sleepTime, SchedulerSPtr scheduler);
    virtual ~SleepyTask();

  protected:
    virtual void run();

  public:
    bool isFinished();
    bool isExecuting();

    int Result;

  private:
    int    m_sleepTime;
  };
}

#endif // ESPINA_SLEEPYWORKER_H
