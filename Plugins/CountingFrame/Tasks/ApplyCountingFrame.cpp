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

// ESPINA
#include "ApplyCountingFrame.h"

#include <Core/Analysis/Query.h>
#include <Core/Analysis/Category.h>
#include <Core/Analysis/Segmentation.h>
#include <CountingFrames/CountingFrame.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/StereologicalInclusion.h>
#include <GUI/Model/SegmentationAdapter.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
ApplyCountingFrame::ApplyCountingFrame(CountingFrame *countingFrame,
                                       SchedulerSPtr scheduler)
: Task(scheduler)
, m_countingFrame(countingFrame)
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

  auto channel       = m_countingFrame->channel();
  auto segmentations = QueryContents::segmentationsOnChannelSample(channel);

  if (!segmentations.isEmpty())
  {
    double taskProgress = 0;
    double inc = 100.0 / segmentations.size();
    auto constraint = m_countingFrame->categoryConstraint();

    for (auto segmentation: segmentations)
    {
      if (!canExecute()) break;

      if(constraint.isEmpty() || (segmentation->category()->classificationName().startsWith(constraint)))
      {
        auto extension = retrieveOrCreateExtension<StereologicalInclusion>(segmentation->extensions());
        extension->addCountingFrame(m_countingFrame);
        extension->evaluateCountingFrame(m_countingFrame);
      }

      taskProgress += inc;

      reportProgress(taskProgress);
    }
  }

  setFinished(canExecute());
}
