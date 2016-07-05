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

#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <GUI/Model/ChannelAdapter.h>

namespace ESPINA
{
  class CoreFactory;

  namespace CF
  {
    class CountingFrame;

    /** \class ApplyCountingFrame
     * \brief Computes the inclusion of all segmentations in the counting frame.
     *
     */
    class ApplyCountingFrame
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

      private:
        CountingFrame   *m_countingFrame;
        CoreFactory     *m_factory;
    };

    using ApplyCountingFramePtr  = ApplyCountingFrame *;
    using ApplyCountingFrameSPtr = std::shared_ptr<ApplyCountingFrame>;

  } // namespace CF
}// namespace ESPINA

#endif // ESPINA_CREATE_COUNTING_FRAME_H
