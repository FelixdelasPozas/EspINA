/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DISTANCEINFORMATIONREPORT_H_
#define DISTANCEINFORMATIONREPORT_H_

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <Dialogs/DistanceInformation/DistanceInformationDialog.h>
#include <Dialogs/DistanceInformation/DistanceInformationOptionsDialog.h>
#include <Support/Report.h>
#include <Support/Context.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Qt
#include <QMutex>
#include <QWaitCondition>

namespace ESPINA
{
  class DistanceComputationManagerThread;

  /** \class DistanceComputationThread
   * \brief Computes the distance between the two given segmentations.
   *
   */
  class DistanceComputationThread
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief DistanceComputationThread class constructor.
       * \param[in] first first segmentation.
       * \param[in] second second segmentation.
       * \param[in] type distance type.
       * \param[in] context application context.
       *
       */
      explicit DistanceComputationThread(SegmentationAdapterPtr first,
                                         SegmentationAdapterPtr second,
                                         const DistanceInformationOptionsDialog::DistanceType type,
                                         Support::Context &context,
                                         DistanceComputationManagerThread *manager);

      /** \brief DistanceComputationThread class virtual destructor.
       *
       */
      virtual ~DistanceComputationThread()
      {};

      const SegmentationAdapterPtr first() const
      { return m_first; }

      const SegmentationAdapterPtr second() const
      { return m_second; }

      const Nm distance() const
      { return m_distance; }

    protected:
      virtual void run() override;

    private:
      /** \brief Returns the centroid for the given segmentation.
       * \param[in] segmentation segmentation pointer.
       *
       */
      const NmVector3 getCentroid(SegmentationAdapterPtr segmentation);

      Support::Context                                    &m_context;  /** application context.            */
      Nm                                                   m_distance; /** distance between segmentations. */
      SegmentationAdapterPtr                               m_first;    /** first segmentation.             */
      SegmentationAdapterPtr                               m_second;   /** second segmentation.            */
      const DistanceInformationOptionsDialog::DistanceType m_type;     /** distance type.                  */
      DistanceComputationManagerThread                    *m_manager;  /** thread manager.                 */
  };

  /** \class DistanceComputationManagerThread
   * \brief Manages the distance computation threads and returns the results.
   *
   */
  class DistanceComputationManagerThread
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief DistanceComputationManagerThread class constructor.
       *
       */
      DistanceComputationManagerThread(const SegmentationAdapterList                   selectedSegmentations,
                                       const DistanceInformationOptionsDialog::Options options,
                                       Support::Context                               &context);

      /** \brief DistanceComputationManagerThread class virtual destructor.
       *
       */
      ~DistanceComputationManagerThread();

      /** \brief Returns the computed distances map.
       *
       */
      DistanceInformationDialog::DistancesMap getDistances() const;

      /** \brief Returns the computation options.
       *
       */
      DistanceInformationOptionsDialog::Options getOptions() const
      { return m_options; }

      /** \brief Returns the list of segmentations to compute distances to or an empty list for all in the model.
       *
       */
      SegmentationAdapterList getSegmentations() const
      { return m_iSegmentations; }

    protected:
      virtual void run() override;

    private slots:
      /** \brief Inserts the computed value into the distances map.
       *
       */
      void onComputationFinished();

    private:
      /** \brief Starts the next computation thread or does nothing if all distances have been computed.
       *
       */
      void computeNextDistance();

      /** \brief Returns true if the distance has been computed already and false otherwise.
       * \param[in] first segmentation.
       * \param[in] second segmentation.
       */
      const bool isDistanceCached(SegmentationAdapterPtr first, SegmentationAdapterPtr second) const;

    private:
      /** \brief Returns true if the segmentation is from the give category or a subcategory if it.
       * \param[in] segmentation segmentation pointer.
       * \param[in] category category smart pointer.
       *
       */
      const bool validCategory(const SegmentationAdapterPtr seg1, const CategoryAdapterSPtr category) const;

      friend class DistanceComputationThread;

      const SegmentationAdapterList                   m_iSegmentations;  /** input segmentations (from).               */
      const SegmentationAdapterList                   m_jSegmentations;  /** input segmentations (to).                 */
      const DistanceInformationOptionsDialog::Options m_options;         /** computation options.                      */
      long int                                        m_i;               /** i iterator for thread launch.             */
      long int                                        m_j;               /** j iterator for thread launch.             */
      long long int                                   m_numComputations; /** total number of computations.             */
      long long int                                   m_computations;    /** number of completed computations so far.  */
      std::atomic<bool>                               m_finished;        /** true if calculations finished completely. */

      Support::Context                                 &m_context;   /** application context.                 */
      mutable QMutex                                    m_lock;      /** lock for the distances.              */
      DistanceInformationDialog::DistancesMap           m_distances; /** distances values.                    */
      QList<std::shared_ptr<DistanceComputationThread>> m_tasks;     /** computation tasks currently running. */

      QWaitCondition m_waiter;    /** wait until all computations have finished. */
      QMutex         m_waitMutex; /** waiter mutex.                              */
  };

  /** \class DistanceInformationReport
   * \brief Distance between segmentations report.
   *
   */
  class DistanceInformationReport
  : public Support::Report
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief DistanceInformationReport class constructor.
       * \param[in] context application context.
       *
       */
      explicit DistanceInformationReport(Support::Context &context);

      /** \brief DistanceInformationReport class virtual destructor.
       *
       */
      virtual ~DistanceInformationReport()
      {};

      virtual QString name() const override;

      virtual QString description() const override;

      virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

      virtual QString requiredInputDescription() const override;

      virtual void show(SegmentationAdapterList input) const override;

    private slots:
      /** \brief Launches the dialog after the distances computations has finished.
       *
       */
      void onComputationFinished();

    private:
      mutable QMutex                                                   m_mutex;   /** for accesing the task list.  */
      mutable QList<std::shared_ptr<DistanceComputationManagerThread>> m_tasks;   /** currently running task list. */
  };
} // namespace ESPINA

#endif // DISTANCEINFORMATIONREPORT_H_
