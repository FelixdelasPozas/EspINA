/*
 
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CHECK_ANALYSIS_H_
#define ESPINA_CHECK_ANALYSIS_H_

// ESPINA
#include <Dialogs/ProblemList/ProblemListDialog.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QDialog>
#include "ui_CheckAnalysisDialog.h"

namespace ESPINA
{
  //------------------------------------------------------------------------
  class CheckAnalysis
  : public QDialog
  , public Ui::CheckAnalysisDialog
  {
    Q_OBJECT
    public:
      /* \brief CheckAnalysis class constructor.
       *
       */
      explicit CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model);

      /* \brief CheckAnalysis virtual destructor.
       *
       */
      virtual ~CheckAnalysis()
      {};

      /* \brief Returns the problem list. To be called after the dialog has executed.
       *
       */
      ProblemList getProblems()
      { return m_problems; }

    protected slots:
      /* \brief Remove task from the list add increase progress bar.
       *
       */
      void finishedTask();

      /* \brief Adds a problem to the problem list.
       *
       */
      void addProblem(struct Problem problem);

    private:
      ProblemList     m_problems;
      QList<TaskSPtr> m_taskList;
      int             m_problemsNum;
  };

  //------------------------------------------------------------------------
  class CheckTask
  : public Task
  {
    Q_OBJECT
    public:
      /* \brief CheckTask class constructor.
       *
       */
      explicit CheckTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
      : Task   {scheduler}
      , m_item {item}
      , m_model{model}
      {
        setHidden(true);
        setDescription("Checking " + item->data().toString()); // for debugging, the user will never see this
      }

      /* \brief CheckTask class virtual destructor.
       *
       */
      virtual ~CheckTask()
      {}

    signals:
      /* \brief Signal emitted when a problem has been found with the item being checked.
       * \param[out] struct Problem, problem description.
       */
      void problem(struct Problem) const;

    protected:
      /* \brief Implements Task::run().
       *
       */
      virtual void run();

    private:
      /* \brief Checks if a segmentation volume is empty, emits problem(struct Problem) if it is.
       *
       */
      void checkVolumeIsEmpty() const;

      /* \brief Checks if a segmentation mesh is empty, emits problem(struct Problem) if it is.
       *
       */
      void checkMeshIsEmpty() const;

      /* \brief Checks if the segmentation has a channel assigned as a location, emits problem(struct Problem) if not.
       *
       */
      void checkSegmentationHasChannel() const;

      /* \brief Checks segmentation relations and emits problem(struct Problem) for each problem found.
       *
       */
      void checkSegmentationRelations() const;

      /* \brief Checks channel relations and emits problem(struct Problem) for each problem found.
       *
       */
      void checkChannelRelations() const;

      /* \brief Checks ViewItem output for existence and emits problem(struct Problem) for each problem found.
       * Returns true if no problem are found, and false otherwise.
       *
       */
      void checkViewItemOutputs() const;

      NeuroItemAdapterSPtr m_item;
      ModelAdapterSPtr     m_model;
  };

} // namespace ESPINA

#endif // ESPINA_CHECK_ANALYSIS_H_
