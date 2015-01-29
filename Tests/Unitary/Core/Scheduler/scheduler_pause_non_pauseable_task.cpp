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

#include <iostream>
#include <unistd.h>

#include <QCoreApplication>
#include <QThread>
 
using namespace ESPINA;
using namespace std;

namespace SPNPT
{
  class OneShotTask
  : public Task
  {
  public:
    explicit OneShotTask(int sleepTime, SchedulerSPtr scheduler)
    : Task(scheduler)
    , m_sleepTime{sleepTime}
    {}

  private:
    virtual void run()
    {
      usleep(m_sleepTime);
    }

  private:
    int m_sleepTime;
  };
}

using namespace SPNPT;

int scheduler_pause_non_pauseable_task(int argc, char** argv)
{
  bool error = false;

  int period = 5000;//0.005 sec

  int sleepTime = 3*period;

  QCoreApplication app(argc, argv);

  auto scheduler   = std::make_shared<Scheduler>(period);
  auto oneShotTask = std::make_shared<OneShotTask>(sleepTime, scheduler);

  oneShotTask->setDescription("Non Pauseable Simple Task");


  Task::submit(oneShotTask);

  usleep(period);

  QObject::connect(oneShotTask->thread(), SIGNAL(destroyed(QObject*)),
                   &app, SLOT(quit()));

  oneShotTask->pause();
  
  usleep(2*sleepTime);
  
  if (!oneShotTask->isPaused())
  {
    std::cerr << "Unexpected task status: should be paused" << std::endl;
    error = true;
  }

  if (oneShotTask->hasFinished())
  {
    std::cerr << "Unexpected task status: should not has finished" << std::endl;
    error = true;
  }

  oneShotTask->resume();

  usleep(period);

  if (oneShotTask->isPaused())
  {
    std::cerr << "Unexpected task status: should not be paused" << std::endl;
    error = true;
  }

  if (!oneShotTask->hasFinished())
  {
    std::cerr << "Unexpected task status: should has finished" << std::endl;
    error = true;
  }
  
  app.exec();
  
  return error;
}