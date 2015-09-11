/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "testing_support_channel_input.h"

#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>

using namespace ESPINA;
using namespace ESPINA::Testing;

using ChannelVolume = SparseVolume<itkVolumeType>;

//----------------------------------------------------------------------------
DummyChannelReader::DummyChannelReader()

: Filter(InputSList(), "DummyChannelReader", SchedulerSPtr())
{
}

//----------------------------------------------------------------------------
void DummyChannelReader::execute()
{
  Bounds bounds{-0.5, 9.5, -0.5, 9.5,-0.5, 9.5};

  auto data = std::make_shared<ChannelVolume>(bounds);
  data->setBackgroundValue(50);

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, NmVector3{1,1,1});
  }

  m_outputs[0]->setData(data);
  m_outputs[0]->clearEditedRegions();
}

//----------------------------------------------------------------------------
InputSPtr ESPINA::Testing::channelInput()
{

  auto filter = std::make_shared<DummyChannelReader>();
  filter->update();

  return getInput(filter, 0);
}
