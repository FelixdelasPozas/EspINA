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

#ifndef GUI_REPRESENTATIONS_REPRESENTATIONPARALLELUPDATER_H_
#define GUI_REPRESENTATIONS_REPRESENTATIONPARALLELUPDATER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationUpdater.h>

namespace ESPINA
{
  /** \class ParallelUpdaterTask
   * \brief Representation updater task to work in conjunction with RepresentationParallelUpdater class and
   * other objects of the same class to create/modify actors in parallel.
   *
   */
  class ParallelUpdaterTask
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief ParallelUpdaterTask class constructor.
       * \param[in] updateList list of items to create or update.
       * \param[in] settings representation settins.
       * \param[in] scheduler task scheduler.
       * \param[in] pipeline generator of the actors of the items.
       *
       */
      explicit ParallelUpdaterTask(const RepresentationPipeline::ActorsMap &updateList,
                                   const RepresentationState               &settings,
                                   SchedulerSPtr                            scheduler,
                                   RepresentationPipelineSPtr               pipeline);

      /** \brief ParallelUpdaterTask class virtual destructor.
       *
       */
      virtual ~ParallelUpdaterTask()
      {}

      /** \brief Returns the generated/modified actors.
       *
       */
      const RepresentationPipeline::ActorsMap actors() const;

    signals:
      void finished(ParallelUpdaterTask *task);

      void progress(ParallelUpdaterTask *task, int progress);

    protected:
      virtual void run();

    private:
      /** \brief Returns the pipeline to be used to create actors for the given item.
       * \param[in] item view item adapter.
       *
       */
      RepresentationPipelineSPtr sourcePipeline(ViewItemAdapterPtr item) const;

      const RepresentationPipeline::ActorsMap &m_updateList; /** list of items to create/modify actors. */
      const RepresentationState               &m_settings;   /** representation settings.               */
      RepresentationPipelineSPtr               m_pipeline;   /** object that creates/modifies actors.   */
      RepresentationPipeline::ActorsMap        m_actors;     /** generated actors.                      */

  };

  /** \class RepresentationParallelUpdater
   * \brief Representation updater that generates/modifies actors in parallel.
   *
   */
  class EspinaGUI_EXPORT RepresentationParallelUpdater
  : public RepresentationUpdater
  {
      Q_OBJECT
    public:
      /** \brief RepresentationParallelUpdater class constructor.
       * \param[in] scheduler task scheduler.
       * \param[in] pipeline generator of the actors of the items.
       *
       */
      explicit RepresentationParallelUpdater(SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

      /** \brief RepresentationParallelUpdater class virtual destructor.
       *
       */
      virtual ~RepresentationParallelUpdater();

    protected:
      virtual void run();

    private slots:
      /** \brief Gets the results of a finished updater task and wakes this thread if finished.
       * \param[in] task ParallelUpaterTask that has finished or has been cancelled.
       *
       */
      void onTaskFinished(ParallelUpdaterTask *task);

      /** \brief Computes the overall progress and emits the progress signal.
       * \param[in] task task reporting the progress.
       * \param[in] progress progress value in [0-100]
       *
       */
      void computeProgress(ParallelUpdaterTask *task, int progress);

    private:
      void onAbort() override
      { m_condition.wakeAll(); }

      /** \brief Aborts all the running tasks.
       *
       */
      void abortTasks();

      /** \brief Creates a task, inserts it in the task list and launches it.
       * \param[in] inputData input data of the task.
       *
       */
      void createTask(const RepresentationPipeline::ActorsMap &inputData);

      struct Data
      {
          std::shared_ptr<ParallelUpdaterTask> Task;
          int                                  Progress;
      };

      QMutex                                   m_dataMutex; /** thread data mutex.       */
      QMutex                                   m_waitMutex; /** wait condition mutex.    */
      QWaitCondition                           m_condition; /** wait condition.          */
      int                                      m_taskNum;   /** number of initial tasks. */
      QMap<ParallelUpdaterTask *, struct Data> m_tasks;     /** running tasks list.      */
  };

} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_REPRESENTATIONPARALLELUPDATER_H_
