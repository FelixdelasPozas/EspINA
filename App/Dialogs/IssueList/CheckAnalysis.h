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
#include <Extensions/Issues/Issues.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Core/Analysis/Segmentation.h>

// Qt
#include <QDialog>

namespace ESPINA
{
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

  protected:
    void reportIssue(NeuroItemAdapterSPtr item,
                     const Extensions::Issue::Severity &severity,
                     const QString &description,
                     const QString &suggestion) const;

    void reportIssue(NeuroItemAdapterPtr item, Extensions::IssueSPtr issue) const;

    QString deleteHint(NeuroItemAdapterSPtr item) const;

  signals:
    /** \brief Signal emitted when a issue has been found with the item being checked.
     * \param[out] issue found
     */
    void issueFound(Extensions::IssueSPtr issue) const;

  protected:
    ModelAdapterSPtr m_model;
  };

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
      void issuesFound(Extensions::IssueList issues);

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
      void addIssue(Extensions::IssueSPtr issue);

    private:
      using CheckList = QList<std::shared_ptr<CheckTask>>;
      QMutex    m_progressMutex;
      CheckList m_checkList;
      int       m_finishedTasks;

      Extensions::IssueList m_issues;
  };

  //------------------------------------------------------------------------
  class CheckDataTask: public CheckTask
  {
    public:
      /** \brief CheckDataTask class constructor.
       * \param[in] scheduler scheduler smart pointer.
       * \param[in] item item whose datas will be checked.
       * \param[in] mode model adapter smart pointer.
       *
       */
      explicit CheckDataTask(SchedulerSPtr scheduler, NeuroItemAdapterSPtr item, ModelAdapterSPtr model);

      /** \brief CheckDataTask class virtual destructor.
       *
       */
      virtual ~CheckDataTask()
      {};

    protected:
      /** \brief Checks if a data object has valid bounds.
       *
       */
      template<typename T> void checkDataBounds(Output::ReadLockData<T> &data) const;

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
       * \param[in] scheduler to launch the task
       * \param[in] item that will be tested.
       * \param[in] model containing the item.
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

      /** \brief Checks the segmentation's relationships.
       *
       */
      void checkRelations() const;

      /** \brief Checks if the segmentation is associated to a channel.
       *
       */
      void checkHasChannel() const;

      /** \brief Checks that the segmentation is inside the channel's bounds.
       *
       */
      void checkIsInsideChannel(ChannelAdapterPtr channel) const;

    private:
      SegmentationAdapterSPtr m_segmentation; /** segmentation to check. */
  };

  //------------------------------------------------------------------------
  class CheckChannelTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckChannelTask class constructor.
       * \param[in] scheduler scheduler smart pointer.
       * \param[in] item neuro item adapter smart pointer that will be tested.
       * \param[in] model model adapter smart pointer containing the item.
       *
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

      /** \brief Checks the channel's relations.
       *
       */
      void checkRelations() const;

    private:
      ChannelAdapterSPtr m_channel; /** channel to check. */
  };

  //------------------------------------------------------------------------
  class CheckSampleTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckSampleTask class constructor.
       * \param[in] scheduler to launch the task
       * \param[in] item that will be tested.
       * \param[in] model containing the item.
       *
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
      SampleAdapterSPtr m_sample; /** sample to check. */
  };

  //------------------------------------------------------------------------
  class CheckDuplicatedSegmentationsTask
  : public CheckTask
  {
    public:
      /** \brief CheckDuplicatedSegmentationsTask class constructor.
       *
       *
       */
      explicit CheckDuplicatedSegmentationsTask(SchedulerSPtr scheduler, ModelAdapterSPtr model);

      /** \brief CheckDuplicatedSegmentationsTask class virtual destructor.
       *
       */
      ~CheckDuplicatedSegmentationsTask()
      {}

    private:
      virtual void run() override final;

      Extensions::IssueSPtr possibleDuplication(SegmentationAdapterPtr original, SegmentationAdapterPtr duplicated) const;
  };

} // namespace ESPINA

#endif // ESPINA_CHECK_ANALYSIS_H_
