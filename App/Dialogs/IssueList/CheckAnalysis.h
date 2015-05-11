/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/IssueList/IssueListDialog.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  //------------------------------------------------------------------------
  class CheckAnalysis
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief CheckAnalysis class constructor.
       * \param[in] scheduler scheduler smart pointer.
       * \param[in] mode model adapter smart pointer.
       *
       */
      explicit CheckAnalysis(SchedulerSPtr scheduler, ModelAdapterSPtr model);

      /** \brief CheckAnalysis virtual destructor.
       *
       */
      virtual ~CheckAnalysis();

    signals:
      void issuesFound(IssueList issues);

    protected:
      virtual void run() override final;

    protected slots:
      /** \brief Remove task from the list add increase progress value.
       *
       */
      void finishedTask();

      /** \brief Adds a issue to the issue list.
       *
       */
      void addIssue(Issue issue);

    private:
      IssueList       m_issues;
      QList<TaskSPtr> m_taskList;
      int             m_issuesNum;
      int             m_totalTasks;
      int             m_finishedTasks;
  };

  //------------------------------------------------------------------------
  class CheckTask
  : public Task
  {
    Q_OBJECT
    public:
      /** \brief CheckTask class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       * \param[in] item, neuro item adapter smart pointer that will be tested.
       * \param[in] model, model adapter smart pointer containing the item.
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

      /** \brief CheckTask class virtual destructor.
       *
       */
      virtual ~CheckTask()
      {}

    signals:
      /** \brief Signal emitted when a issue has been found with the item being checked.
       * \param[out] issue issue struct.
       */
      void issue(Issue issue) const;

    protected:
      virtual void run() override final;

    private:
      /** \brief Checks if a segmentation volume is empty, emits issue(Issue) if it is.
       *
       */
      void checkVolumeIsEmpty() const;

      /** \brief Checks if a segmentation mesh is empty, emits issue(Issue) if it is.
       *
       */
      void checkMeshIsEmpty() const;

      /** \brief Checks if the segmentation has a channel assigned as a location, emits issue(Issue) if not.
       *
       */
      void checkSegmentationHasChannel() const;

      /** \brief Checks segmentation relations and emits issue(Issue) for each problem found.
       *
       */
      void checkSegmentationRelations() const;

      /** \brief Checks channel relations and emits issue(Issue) for each problem found.
       *
       */
      void checkChannelRelations() const;

      /** \brief Checks ViewItem output for existence and emits issue(Issue) for each problem found.
       * Returns true if no problem are found, and false otherwise.
       *
       */
      void checkViewItemOutputs() const;

      NeuroItemAdapterSPtr m_item;
      ModelAdapterSPtr     m_model;
  };

} // namespace ESPINA

#endif // ESPINA_CHECK_ANALYSIS_H_
