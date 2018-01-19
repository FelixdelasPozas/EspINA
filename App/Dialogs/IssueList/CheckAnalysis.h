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
#include <Support/Context.h>

namespace ESPINA
{
  /** \class CheckTask
   * \brief Class to make some item checks on a separate thread.
   *
   */
  class CheckTask
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief CheckTask class constructor.
       * \param[in] context application context reference.
       *
       */
      explicit CheckTask(Support::Context &context)
      : Task     {context.scheduler()}
      , m_context(context)
      {
        setHidden(true);
      }

      /** \brief CheckTask class virtual destructor.
       *
       */
      virtual ~CheckTask()
      {}

    protected:
      /** \brief Creates the item issue extension, and reports it.
       * \param[in] item item with the issue.
       * \param[in] severity severity value of the issue.
       * \param[in] description issue description.
       * \param[in] suggestion issue solution suggestion.
       *
       * NOTE: doesn't add the issue extension to the item.
       *
       */
      void reportIssue(NeuroItemAdapterSPtr item,
                       const Extensions::Issue::Severity &severity,
                       const QString &description,
                       const QString &suggestion) const;

      /** \brief Adds the issue extension to the item if it's a segmentation and emits the signal.
       * \param[in] item item with the issue.
       * \param[in] issue issue extension.
       *
       */
      void reportIssue(NeuroItemAdapterPtr item, Extensions::IssueSPtr issue) const;

      /** \brief Helper method to create a suggestion depending on the item with the issue.
       * \param[in] item item with the issue.
       *
       */
      QString deleteHint(NeuroItemAdapterSPtr item) const;

      /** \brief Helper method to create a suggestion depending on the item with the issue.
       * \param[in] item item with the issue.
       *
       */
      QString editOrDeleteHint(NeuroItemAdapterSPtr item) const;

    signals:
      /** \brief Signal emitted when a issue has been found with the item being checked.
       * \param[out] issue found
       */
      void issueFound(Extensions::IssueSPtr issue) const;

    protected:
      Support::Context &m_context; /** application context. */
  };

  /** \class CheckAnalysis
   * \brief Task to check the current analysis consistency and report errors/problems.
   *
   */
  class CheckAnalysis
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief CheckAnalysis class constructor.
       * \param[in] context application context reference.
       *
       */
      explicit CheckAnalysis(Support::Context &context);

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
      /** \brief Removes the issues extensions from all elements of the given model.
       * \param[in] model analysis model.
       *
       */
      void removePreviousIssues(ModelAdapterSPtr model);

    private:
      using CheckList = QList<std::shared_ptr<CheckTask>>;

      QMutex    m_progressMutex; /** mutex to protect the progression value. */
      CheckList m_checkList;     /** list of check tasks.                    */
      int       m_finishedTasks; /** number of finished check tasks.         */

      Extensions::IssueList m_issues; /** list of reported issues. */
  };

  /** \class CheckDataTask
   * \brief Task to check for errors/problems in the data of the items.
   *
   */
  class CheckDataTask
  : public CheckTask
  {
    public:
      /** \brief CheckDataTask class constructor.
       * \param[in] context application context reference.
       * \param[in] item item whose datas will be checked.
       *
       */
      explicit CheckDataTask(Support::Context &context, NeuroItemAdapterSPtr item);

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

      /** \brief Checks the validity of the extensions of the item.
       *
       */
      virtual void checkExtensionsValidity() const = 0;

      /** \brief Checks ViewItem output for existence and emits issue(Issue) for each problem found.
       * Returns true if no problem are found, and false otherwise.
       *
       */
      void checkViewItemOutputs(ViewItemAdapterSPtr viewItem) const;

      NeuroItemAdapterSPtr m_item; /** item to check for problems in the data. */
  };

  /** \class CheckSegmentationTask
   * \brief Task to check for errors/problems in a segmentation.
   *
   */
  class CheckSegmentationTask
  : public CheckDataTask
  {
    Q_OBJECT

    public:
      /** \brief CheckSegmentationTask class constructor.
       * \param[in] context application context reference.
       * \param[in] item that will be tested.
       */
      explicit CheckSegmentationTask(Support::Context &context, NeuroItemAdapterSPtr item);

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

      virtual void checkExtensionsValidity() const override final;

      /** \brief Checks the segmentation's relationships.
       *
       */
      void checkRelations() const;

      /** \brief Checks if the segmentation is associated to a channel.
       *
       */
      void checkHasChannel() const;

      /** \brief Checks that the segmentation is inside the channel's bounds.
       * \param[in] stack stack item.
       *
       */
      void checkIsInsideChannel(ChannelAdapterPtr stack) const;

      /** \brief Checks for problems in the skeleton not related to the data but to itself, like loops or connections.
       *
       */
      void checkSkeletonProblems() const;

    private:
      SegmentationAdapterSPtr m_segmentation; /** segmentation to check. */
  };

  /** \class CheckStackTask
   * \brief Task to check for errors/problems in a stack.
   *
   */
  class CheckStackTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckStackTask class constructor.
       * \param[in] context application context reference.
       * \param[in] item neuro item adapter smart pointer that will be tested.
       *
       */
      explicit CheckStackTask(Support::Context &context, NeuroItemAdapterSPtr item);

      /** \brief CheckStackTask class virtual destructor.
       *
       */
      virtual ~CheckStackTask()
      {};

    protected:
      virtual void run() override final;

      virtual void checkVolumeIsEmpty() const override final;

      virtual void checkMeshIsEmpty() const override final
      {};

      virtual void checkSkeletonIsEmpty() const override final
      {};

      virtual void checkExtensionsValidity() const override final;

      /** \brief Checks the stack's relations.
       *
       */
      void checkRelations() const;

    private:
      ChannelAdapterSPtr m_stack; /** stack to check. */
  };

  /** \class CheckSampleTask
   * \brief Task to check for errors/problems in a sample.
   *
   */
  class CheckSampleTask
  : public CheckDataTask
  {
      Q_OBJECT
    public:
      /** \brief CheckSampleTask class constructor.
       * \param[in] context application context reference.
       * \param[in] item that will be tested.
       *
       */
      explicit CheckSampleTask(Support::Context &context, NeuroItemAdapterSPtr item);

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

      virtual void checkExtensionsValidity() const override final
      {};

    private:
      SampleAdapterSPtr m_sample; /** sample to check. */
  };

  /** \class CheckDuplicatedSegmentationsTask
   * \brief Task to check for duplicated segmentations.
   *
   */
  class CheckDuplicatedSegmentationsTask
  : public CheckTask
  {
    public:
      /** \brief CheckDuplicatedSegmentationsTask class constructor.
       * \param[in] context application context reference.
       *
       */
      explicit CheckDuplicatedSegmentationsTask(Support::Context &context);

      /** \brief CheckDuplicatedSegmentationsTask class virtual destructor.
       *
       */
      ~CheckDuplicatedSegmentationsTask()
      {}

    private:
      virtual void run() override final;

      /** \brief Returns an issue reporting the possible duplication of 'original' segmentation by 'duplicated' segmentation.
       * \param[in] original segmentation.
       * \param[in] duplicated segmentation.
       * \param[in] duplicatedVoxels number of common voxels.
       *
       */
      Extensions::IssueSPtr possibleDuplication(SegmentationAdapterPtr original, SegmentationAdapterPtr duplicated, const unsigned long long duplicatedVoxels) const;

      SegmentationAdapterSList m_segmentations; /** list of segmentations to check for duplicates. */
  };

} // namespace ESPINA

#endif // ESPINA_CHECK_ANALYSIS_H_
