/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#include "QueryAdapter.h"

#include <Core/Analysis/Query.h>
#include <GUI/Model/ModelAdapter.h>

using namespace EspINA;

//------------------------------------------------------------------------
ChannelAdapterSList QueryAdapter::channels(SegmentationAdapterSPtr segmentation)
{
  return channels(segmentation.get());
}

//------------------------------------------------------------------------
ChannelAdapterSList QueryAdapter::channels(SegmentationAdapterPtr segmentation)
{
  auto adaptedSegmentation = segmentation->m_segmentation;
  auto model = segmentation->model();

  ChannelAdapterSList adaptedChannels;

  for (auto adaptedChannel : Query::channels(adaptedSegmentation))
  {
    for (auto channel : model->channels())
    {
      if (channel->m_channel == adaptedChannel)
      {
        adaptedChannels << channel;
      }
    }
  }

  return adaptedChannels;
}

//------------------------------------------------------------------------
SampleAdapterSPtr QueryAdapter::sample(ChannelAdapterSPtr channel)
{
  return sample(channel.get());
}

//------------------------------------------------------------------------
SampleAdapterSPtr QueryAdapter::sample(ChannelAdapterPtr channel)
{
  auto adaptedChannel = channel->m_channel;
  auto model = channel->model();

  auto adaptedSample = Query::sample(adaptedChannel);
  for (auto sample : model->samples())
  {
    if (sample->m_sample == adaptedSample)
    {
      return sample;
    }
  }

  return SampleAdapterSPtr();
}

