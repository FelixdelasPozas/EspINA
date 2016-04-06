/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// Plugin
#include <CountingFrames/AdaptiveCountingFrame.h>
#include <CountingFrames/OrthogonalCountingFrame.h>
#include <CountingFrames/CountingFrame.h>
#include <Tasks/CountingFrameCreator.h>

using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFrameCreator::CountingFrameCreator(Data data, SchedulerSPtr scheduler)
: Task  {scheduler}
, m_cf  {nullptr}
{
  m_data = data;
}

//------------------------------------------------------------------------
CountingFrameCreator::~CountingFrameCreator()
{
}

//------------------------------------------------------------------------
void CountingFrameCreator::run()
{
  reportProgress(25);

  Nm inclusion[3];
  Nm exclusion[3];
  for(int i = 0; i < 3; ++i)
  {
    inclusion[i] = m_data.inclusion[i];
    exclusion[i] = m_data.exclusion[i];
  }

  switch(m_data.type)
  {
    case CFType::ORTOGONAL:
      m_cf = OrthogonalCountingFrame::New(m_data.extension, inclusion, exclusion, m_scheduler);
      break;
    case CFType::ADAPTIVE:
      m_cf = AdaptiveCountingFrame::New(m_data.extension, inclusion, exclusion, m_scheduler);
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  reportProgress(100);
}

//------------------------------------------------------------------------
CountingFrame* CountingFrameCreator::getCountingFrame() const
{
  return m_cf;
}

//------------------------------------------------------------------------
CountingFrameCreator::Data CountingFrameCreator::getCountingFrameData() const
{
  return m_data;
}
