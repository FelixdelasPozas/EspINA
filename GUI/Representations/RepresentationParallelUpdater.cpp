/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <Core/MultiTasking/Scheduler.h>
#include <GUI/Representations/Frame.h>
#include <GUI/Representations/RepresentationParallelUpdater.h>

// Qt
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//--------------------------------------------------------------------
ParallelUpdaterTask::ParallelUpdaterTask(const RepresentationPipeline::ActorsMap& updateList,
                                         const RepresentationState&               settings,
                                         SchedulerSPtr                            scheduler,
                                         RepresentationPipelineSPtr               pipeline)
: Task        {scheduler}
, m_updateList(updateList)
, m_settings  (settings)
, m_pipeline  {pipeline}
{
  setDescription(tr("Parallel Updater Task"));
  setHidden(true);
}

//--------------------------------------------------------------------
const RepresentationPipeline::ActorsMap ParallelUpdaterTask::actors() const
{
  return m_actors;
}

//--------------------------------------------------------------------
void ParallelUpdaterTask::run()
{
  auto it   = m_updateList.begin();
  auto size = m_updateList.size();
  int i = 0;

  while (canExecute() && it != m_updateList.end())
  {
    auto item     = it.key();
    auto actors   = it.value();
    auto pipeline = sourcePipeline(item);
    auto state    = pipeline->representationState(item, m_settings);

    if (actors.empty())
    {
      m_actors[item] = pipeline->createActors(item, state);
    }
    else
    {
      m_actors[item] = actors;
    }

    pipeline->updateColors(m_actors[item], item, state);

    ++it;
    ++i;

    emit progress(this, (i * 100)/size);
  }

  emit finished(this);
}

//----------------------------------------------------------------------------
RepresentationPipelineSPtr ParallelUpdaterTask::sourcePipeline(ViewItemAdapterPtr item) const
{
  auto pipeline = item->temporalRepresentation();

  if (!pipeline || pipeline->type() != m_pipeline->type())
  {
    pipeline = m_pipeline;
  }

  return pipeline;
}

//--------------------------------------------------------------------
RepresentationParallelUpdater::RepresentationParallelUpdater(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline)
: RepresentationUpdater{scheduler, pipeline}
, m_taskNum            {0}
{
  setDescription(tr("Representation Parallel Updater"));
  setHidden(true);
}

//--------------------------------------------------------------------
RepresentationParallelUpdater::~RepresentationParallelUpdater()
{
  abortTasks();
}

//--------------------------------------------------------------------
void RepresentationParallelUpdater::run()
{
  auto frame = std::make_shared<Frame>();
  RepresentationState settings;
  UpdateRequestList updateList;

  {
    QReadLocker lock(&m_dataLock);
    frame->time = m_frame->time;
    frame->crosshair = m_frame->crosshair;
    frame->resolution = m_frame->resolution;
    frame->bounds = m_frame->bounds;
    frame->flags = m_frame->flags;
    settings = m_settings;
  }

  {
    QWriteLocker lock(&m_dataLock);

    // Local copy needed to prevent condition race on same frame
    // (usually due to invalidation view item representations)
    updateList = *m_updateList;
    updateList.detach();

    m_updateList    = &m_sources;
    m_requestedSources.clear();
  }

  if(updateList.isEmpty()) return;

  auto size     = updateList.size();
  auto maxTasks = static_cast<int>(Scheduler::maxRunningTasks());

  RepresentationPipeline::ActorsMap data[maxTasks];

  {
    RepresentationPipeline::ActorsLocker actors(m_actors);
    QMutexLocker dataLock(&m_dataMutex);

    if(size < maxTasks)
    {
      for(auto request: updateList)
      {
        if(!canExecute()) break;
        data[0][request.first] = (request.second == false ? actors.get()[request.first] : RepresentationPipeline::ActorList());
      }

      createTask(data[0], settings);
    }
    else
    {
      int i = 0;

      // partition the data.
      for(auto request: updateList)
      {
        if(!canExecute()) break;
        data[i % maxTasks][request.first] = (request.second == false ? actors.get()[request.first] : RepresentationPipeline::ActorList());
        ++i;
      }

      for(i = 0; i < maxTasks && canExecute(); ++i)
      {
        createTask(data[i], settings);
      }
    }
  }

  if(canExecute())
  {
    m_waitMutex.lock();
    m_condition.wait(&m_waitMutex);
    m_waitMutex.unlock();
  }

  if(!canExecute())
  {
    abortTasks();
  }
  else
  {
    if(isValid(frame))
    {
      emit actorsReady(frame, m_actors);
    }
  }
}

//--------------------------------------------------------------------
void RepresentationParallelUpdater::onTaskFinished(ParallelUpdaterTask* task)
{
  QMutexLocker lock(&m_dataMutex);

  if(!canExecute())
  {
    m_condition.wakeAll();
    return;
  }

  {
    RepresentationPipeline::ActorsLocker finalActors(m_actors);
    auto actors = task->actors();

    for(auto item: actors.keys())
    {
      finalActors.get()[item] = actors[item];
    }
  }

  m_tasks.remove(task);

  if(m_tasks.isEmpty())
  {
    m_condition.wakeAll();
  }
}

//--------------------------------------------------------------------
void RepresentationParallelUpdater::abortTasks()
{
  QMutexLocker lock(&m_dataMutex);

  for(auto task: m_tasks)
  {
    disconnect(task.Task.get(), SIGNAL(finished(ParallelUpdaterTask *)), this, SLOT(onTaskFinished(ParallelUpdaterTask *)));
    disconnect(task.Task.get(), SIGNAL(progress(ParallelUpdaterTask *, int)), this, SLOT(computeProgress(ParallelUpdaterTask *, int)));

    task.Task->abort();

    if(!task.Task->thread()->wait(500))
    {
      task.Task->thread()->terminate();
    }
  }

  m_tasks.clear();
}

//--------------------------------------------------------------------
void RepresentationParallelUpdater::createTask(const RepresentationPipeline::ActorsMap& inputData, const RepresentationState &settings)
{
  auto task = std::make_shared<ParallelUpdaterTask>(inputData, settings, m_scheduler, m_pipeline);

  connect(task.get(), SIGNAL(finished(ParallelUpdaterTask *)), this, SLOT(onTaskFinished(ParallelUpdaterTask *)), Qt::DirectConnection);
  connect(task.get(), SIGNAL(progress(ParallelUpdaterTask *, int)), this, SLOT(computeProgress(ParallelUpdaterTask *, int)), Qt::DirectConnection);

  struct Data data;
  data.Task     = task;
  data.Progress = 0;

  m_tasks[task.get()] = data;

  Task::submit(task);
}

//--------------------------------------------------------------------
void RepresentationParallelUpdater::computeProgress(ParallelUpdaterTask *task, int progress)
{
  QMutexLocker lock(&m_dataMutex);

  m_tasks[task].Progress = progress;

  long int totalProgress = 0;

  for(auto data: m_tasks)
  {
    totalProgress += data.Progress;
  }

  reportProgress(totalProgress/m_tasks.size());
}
