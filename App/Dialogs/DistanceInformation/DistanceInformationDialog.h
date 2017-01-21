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

#ifndef DISTANCEINFORMATIONDIALOG_H_
#define DISTANCEINFORMATIONDIALOG_H_

// Qt
#include <QDebug>
#include <QDialog>
#include <QThread>

// ESPINA
#include "DistanceInformationOptionsDialog.h"
#include <Core/Types.h>
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class QProgressBar;

namespace ESPINA
{
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
                                         Support::Context &context);

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
  };

  /** \class DistanceInformationDialog
    * \brief Shows the report dialog.
    *
    */
  class DistanceInformationDialog
  : public QDialog
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief DistanceInformationDialog class constructor.
       * \param[in] parent widget.
       * \param[in] parent widget.
       */
      explicit DistanceInformationDialog(const SegmentationAdapterList selectedSegmentations,
                                         const DistanceInformationOptionsDialog::Options options,
                                         Support::Context &context);

      /** \brief DistanceInformationDialog class destructor.
       *
       */
      virtual ~DistanceInformationDialog()
      {};

      using DistancesMap = QMap<SegmentationAdapterPtr, QMap<SegmentationAdapterPtr, double>>;

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

    private slots:
      /** \brief Cancels the computations and quits the dialog.
       *
       */
      void onComputationCancelled();

      /** \brief Inserts the computed value into the distances map.
       *
       */
      void onComputationFinished();

    private:
      const SegmentationAdapterList                   &m_segmentations; /** input segmentations.           */
      const DistanceInformationOptionsDialog::Options &m_options;       /** computation options.           */
      int                                              m_i;             /** i iterator for thread launch.  */
      int                                              m_j;             /** j iterator for thread launch.  */
      const int                                        m_iMax;          /** max i iterator value.          */
      const int                                        m_jMax;          /** max j iterator value.          */
      bool                                             m_finished;      /** true if calculations finished. */

      mutable QReadWriteLock                            m_lock;      /** lock for the distances.              */
      DistancesMap                                      m_distances; /** distances map                        */
      QList<std::shared_ptr<DistanceComputationThread>> m_tasks;     /** computation tasks currently running. */
      QProgressBar                                     *m_progress;  /** computation progress bar.            */
  };

} // namespace ESPINA

#endif /* ESPINA_DISTANCEINFORMATIONDIALOG_H_ */
