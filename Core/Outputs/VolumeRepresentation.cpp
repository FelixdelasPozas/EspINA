/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "VolumeRepresentation.h"

using namespace EspINA;


//----------------------------------------------------------------------------
const FilterOutput::OutputRepresentationName ChannelVolume::TYPE = "ChannelVolumeType";

//----------------------------------------------------------------------------
ChannelVolumePtr EspINA::channelVolume(OutputPtr output)
{
  ChannelOutputPtr channelOutput = dynamic_cast<ChannelOutputPtr>(output);
  Q_ASSERT(channelOutput);
  return dynamic_cast<ChannelVolume *>(channelOutput->representation(ChannelVolume::TYPE).get());
}

//----------------------------------------------------------------------------
ChannelVolumeSPtr EspINA::channelVolume(OutputSPtr output)
{
  ChannelOutputSPtr channelOutput = boost::dynamic_pointer_cast<ChannelOutput>(output);
  Q_ASSERT(channelOutput.get());
  return boost::dynamic_pointer_cast<ChannelVolume>(channelOutput->representation(ChannelVolume::TYPE));
}



//----------------------------------------------------------------------------
const FilterOutput::OutputRepresentationName SegmentationVolume::TYPE = "RawSegmentationVolume";

//----------------------------------------------------------------------------
SegmentationVolumePtr EspINA::segmentationVolume(OutputPtr output)
{
  SegmentationOutputPtr segmentationOutput = dynamic_cast<SegmentationOutputPtr>(output);
  Q_ASSERT(segmentationOutput);
  return dynamic_cast<SegmentationVolume *>(segmentationOutput->representation(SegmentationVolume::TYPE).get());
}

//----------------------------------------------------------------------------
SegmentationVolumeSPtr EspINA::segmentationVolume(OutputSPtr output)
{
  SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
  Q_ASSERT(segmentationOutput.get());
  return boost::dynamic_pointer_cast<SegmentationVolume>(segmentationOutput->representation(SegmentationVolume::TYPE));
}

//----------------------------------------------------------------------------
SegmentationVolumeSPtr EspINA::segmentationVolume(SegmentationOutputSPtr output)
{
  return boost::dynamic_pointer_cast<SegmentationVolume>(output->representation(SegmentationVolume::TYPE));
}