/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_APPLY_COUNTING_FRAME_H
#define ESPINA_APPLY_COUNTING_FRAME_H

#include "CountingFramePlugin_Export.h"

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <GUI/Model/ChannelAdapter.h>

// Qt
#include <QMutex>
#include <QWaitCondition>

namespace ESPINA
{
  class CoreFactory;

  namespace CF
  {
    class CountingFrame;

    /** \class ApplySegmentationsCountingFrame
     * \brief Applies the counting frame to the given segmentations list.
     *
     */
    class ApplySegmentationCountingFrame
    : public Task
    {
        Q_OBJECT
      public:
        /** \brief ApplySegmentationCountingFrame class constructor.
         * \param[in] countingFrame countingFrame of the segmentations.
         * \param[in] segmentations segmentations list to apply.
         * \param[in] factory core factory for extension creation.
         * \param[in] scheduler application task scheduler.
         *
         */
        ApplySegmentationCountingFrame(CountingFrame    *countingFrame,
                                       SegmentationSList segmentations,
                                       CoreFactory      *factory,
                                       SchedulerSPtr     scheduler);

        /** \brief ApplySegmentationCountingFrame class virtual destructor.
         *
         */
        virtual ~ApplySegmentationCountingFrame()
        {};

      signals:
        void progress(int value, ApplySegmentationCountingFrame* task);

      protected:
        virtual void run() override;

      private:
        CountingFrame    *m_countingFrame; /** CountingFrame of the segmentations.  */
        SegmentationSList m_segmentations; /** list of segmentations to apply.      */
        CoreFactory      *m_factory;       /** core factory for extension creation. */
    };

    /** \class ApplyCountingFrame
     * \brief Computes the inclusion of all segmentations in the counting frame.
     *
     */
    class CountingFramePlugin_EXPORT ApplyCountingFrame
    : public Task
    {
        Q_OBJECT
      public:
        /** \brief ApplyCountingFrame class constructor.
         * \param[in] countingFrame counting frame pointer.
         * \param[in] factory model factory pointer.
         * \param[in] sheduler application task scheduler.
         *
         */
        explicit ApplyCountingFrame(CountingFrame   *countingFrame,
                                    CoreFactory     *factory,
                                    SchedulerSPtr    scheduler);

        /** \brief ApplyCountingFrame class virtual destructor.
         *
         */
        virtual ~ApplyCountingFrame();

      protected:
        virtual void run();

      private slots:
        /** \brief Computes and signals the progress of the whole operation.
         * \param[in] value task progress value.
         * \param[in] task computing task pointer.
         *
         */
        void onTaskProgress(int value, ApplySegmentationCountingFrame *task);

        /** \brief Wakes up the thread when all the sub tasks have finished computation.
         *
         */
        void onTaskFinished();

      private:
        void onAbort() override
        { m_condition.wakeAll(); }

        /** \brief Aborts the computation tasks.
         *
         */
        void abortTasks();

        CountingFrame *m_countingFrame; /** counting frame to apply              */
        CoreFactory   *m_factory;       /** core factory for extension creation. */
        QMutex         m_waitMutex;     /** wait condition mutex.                */
        QWaitCondition m_condition;     /** wait condition to stop the thread.   */

        using TaskType = std::shared_ptr<ApplySegmentationCountingFrame>;

        /** \struct Data
         * \brief Executing tasks context.
         *
         */
        struct Data
        {
            TaskType Task;     /** running task.          */
            int      Progress; /** task current progress. */
        };

        QMap<ApplySegmentationCountingFrame *, struct Data> m_tasks; /** executing task list. */
    };

    using ApplyCountingFramePtr  = ApplyCountingFrame *;
    using ApplyCountingFrameSPtr = std::shared_ptr<ApplyCountingFrame>;

  } // namespace CF
}// namespace ESPINA

#endif // ESPINA_CREATE_COUNTING_FRAME_H
