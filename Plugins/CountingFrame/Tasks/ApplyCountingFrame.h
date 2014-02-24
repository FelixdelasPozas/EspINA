/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <GUI/Model/ChannelAdapter.h>

namespace EspINA
{
  namespace CF
  {
    class CountingFrame;

    class ApplyCountingFrame
    : public Task
    {
    public:
      explicit ApplyCountingFrame(CountingFrame *countingFrame,
                                  SchedulerSPtr scheduler = SchedulerSPtr());
      virtual ~ApplyCountingFrame();

      bool hasBeenLaunched() const
      {
        return m_hasBeenLaunched;
      }

      void restart()
      {
        QMutexLocker lock(&m_mutex);
        m_hasToBeRestarted = true;
      }

    protected:
      virtual void run();

    private:
      CountingFrame *m_countingFrame;

      bool m_hasBeenLaunched; //TODO: Move to Task API

      QMutex m_mutex;
      bool   m_hasToBeRestarted;
    };

    using ApplyCountingFramePtr  = ApplyCountingFrame *;
    using ApplyCountingFrameSPtr = std::shared_ptr<ApplyCountingFrame>;

  } // namespace CF
}// namespace EspINA

#endif // ESPINA_CREATE_COUNTING_FRAME_H
