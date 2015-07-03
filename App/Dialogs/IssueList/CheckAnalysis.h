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
      virtual ~CheckAnalysis()
      {};

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
      QMutex          m_progressMutex;
      IssueList       m_issues;
      QList<TaskSPtr> m_taskList;
      int             m_issuesNum;
      int             m_finishedTasks;
  };

  //------------------------------------------------------------------------
  class CheckTask
  : public Task
  {
    Q_OBJECT
    public:
      /** \brief CheckTask class constructor.
       * \param[in] scheduler smart pointer.
       * \param[in] model adapter smart pointer containing the item.
       *
       */
      explicit CheckTask(SchedulerSPtr scheduler, ModelAdapterSPtr model)
      : Task   {scheduler}
      , m_model{model}
      {
        setHidden(true);
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
      ModelAdapterSPtr m_model;
  };

  class CheckDataTask
  : public CheckTask
  {
  public:
      explicit CheckDataTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model)
      : CheckTask{scheduler, model}
      {
        setDescription("Checking " + item->data().toString()); // for debugging, the user will never see this
      }

    protected:
      /** \brief Checks if a view item is empty, emits issue(Issue) if it is.
       *
       */
      virtual void checkVolumeIsEmpty() const = 0;

      /** \brief Checks if a segmentation mesh is empty, emits issue(Issue) if it is.
       *
       */
      virtual void checkMeshIsEmpty() const = 0;

      /** \brief Checks if a segmentation skeleton is empty, emits issue(Issue) if it is.
       *
       */
      virtual void checkSkeletonIsEmpty() const = 0;

      /** \brief Checks ViewItem output for existence and emits issue(Issue) for each problem found.
       * Returns true if no problem are found, and false otherwise.
       *
       */
      void checkViewItemOutputs(ViewItemAdapterSPtr viewItem) const;

      NeuroItemAdapterSPtr m_item;
  };

  //------------------------------------------------------------------------
  class CheckSegmentationTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckSegmentationTask class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       * \param[in] item, neuro item adapter smart pointer that will be tested.
       * \param[in] model, model adapter smart pointer containing the item.
       */
      explicit CheckSegmentationTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model);

      /** \brief CheckSegmentationTask class virtual destructor.
       *
       */
      virtual ~CheckSegmentationTask()
      {};

    protected:
      virtual void run() override final;

      virtual void checkVolumeIsEmpty() const override final;

      virtual void checkMeshIsEmpty() const override final;

      virtual void checkSkeletonIsEmpty() const override final;

      void checkRelations() const;

      void checkHasChannel() const;

    private:
      SegmentationAdapterSPtr m_segmentation;
  };

  //------------------------------------------------------------------------
  class CheckChannelTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckChannelTask class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       * \param[in] item, neuro item adapter smart pointer that will be tested.
       * \param[in] model, model adapter smart pointer containing the item.
       */
      explicit CheckChannelTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model);

      /** \brief CheckChannelTask class virtual destructor.
       *
       */
      virtual ~CheckChannelTask()
      {};

    protected:
      virtual void run() override final;

      virtual void checkVolumeIsEmpty() const override final;

      virtual void checkMeshIsEmpty() const override final
      {};

      virtual void checkSkeletonIsEmpty() const override final
      {};

      void checkRelations() const;

    private:
      ChannelAdapterSPtr m_channel;
  };

  //------------------------------------------------------------------------
  class CheckSampleTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckSampleTask class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       * \param[in] item, neuro item adapter smart pointer that will be tested.
       * \param[in] model, model adapter smart pointer containing the item.
       */
      explicit CheckSampleTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model);

      /** \brief CheckSampleTask class virtual destructor.
       *
       */
      virtual ~CheckSampleTask()
      {};

    protected:
      virtual void run() override final;

      virtual void checkVolumeIsEmpty() const override final
      {};

      virtual void checkMeshIsEmpty() const override final
      {};

      virtual void checkSkeletonIsEmpty() const override final
      {};

    private:
      SampleAdapterSPtr m_sample;
  };

  //------------------------------------------------------------------------
  class CheckDuplicatedSegmentationsTask
  : public CheckTask
  {
  public:
    explicit CheckDuplicatedSegmentationsTask(SchedulerSPtr scheduler, ModelAdapterSPtr model);

  private:
    virtual void run() override final;

    Issue possibleDuplication(SegmentationAdapterPtr seg1, SegmentationAdapterPtr seg2) const;
  };

} // namespace ESPINA

#endif // ESPINA_CHECK_ANALYSIS_H_
