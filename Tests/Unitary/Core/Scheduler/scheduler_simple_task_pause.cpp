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

#include <QApplication>
#include <QThread>
 
using namespace EspINA;
using namespace std;

int scheduler_simple_task_pause( int argc, char** argv )
{
  int error = 0;
  
  int period = 50000;//0.05 sec
  
  int tasksPerPeriod = 2;
  int sleepTime = period/tasksPerPeriod;
  int taskTime  = 10*sleepTime;
    
  QApplication app(argc, argv);
  
  Scheduler scheduler(period);
  SleepyTask* sleepyTask = new SleepyTask(sleepTime, &scheduler);  
  sleepyTask->setDescription("Simple Task");
  
  if (sleepyTask->Result != -1) {
    error = 1;
    std::cerr << "Unexpected initial sleepy task value" << std::endl;
  }
  
  sleepyTask->submit();
  
  usleep(taskTime/2);
  
  sleepyTask->pause();
  
  usleep(taskTime);
  
  if (sleepyTask->Result != 0) {
    error = 1;
    std::cerr << "Unexpected paused sleepy task value" << std::endl;
  }
  
  sleepyTask->resume();
  
  usleep(taskTime);
  
  if (sleepyTask->Result != 1) {
    error = 1;
    std::cerr << "Unexpected final sleepy task value" << std::endl;
  }
  
  QObject::connect(sleepyTask->thread(), SIGNAL(destroyed(QObject*)),
                   &app, SLOT(quit()));
  
  sleepyTask->deleteLater();
  
  app.exec();
  
  return error;
}