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

// EspINA
#include "ApplyCountingFrame.h"

#include <Core/Analysis/Query.h>
#include <Core/Analysis/Segmentation.h>
#include <CountingFrames/CountingFrame.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/StereologicalInclusion.h>
#include <GUI/Model/SegmentationAdapter.h>

#include <QDebug>

using namespace EspINA;
using namespace EspINA::CF;

//------------------------------------------------------------------------
ApplyCountingFrame::ApplyCountingFrame(CountingFrame *countingFrame,
                                       SchedulerSPtr scheduler)
: Task(scheduler)
, m_countingFrame(countingFrame)
, m_hasBeenLaunched(false)
, m_hasToBeRestarted(true)
{
}

//------------------------------------------------------------------------
ApplyCountingFrame::~ApplyCountingFrame()
{
}

//------------------------------------------------------------------------
void ApplyCountingFrame::run()
{
  setDescription(tr("Applying %1 CF").arg(m_countingFrame->id()));

  m_hasBeenLaunched = true;

  while (m_hasToBeRestarted && canExecute())
  {
    {
      QMutexLocker lock(&m_mutex);
      m_hasToBeRestarted = false;
    }

    auto channel       = m_countingFrame->channel();
    auto segmentations = Query::segmentationsOnChannelSample(channel);

    double taskProgress = 0;
    double inc = 100.0 / segmentations.size();

    for (auto segmentation : segmentations)
    {
      if (!canExecute() || m_hasToBeRestarted) break;

      auto extension = retrieveOrCreateExtension<StereologicalInclusion>(segmentation);
      extension->addCountingFrame(m_countingFrame);
      extension->evaluateCountingFrame(m_countingFrame);

      taskProgress += inc;

      emit progress(taskProgress);
    }
  }

  m_hasToBeRestarted = true;
  m_hasBeenLaunched  = false;
}