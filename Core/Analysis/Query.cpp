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

#include "Query.h"
#include "Sample.h"
#include "Analysis.h"
#include "Channel.h"
#include "Segmentation.h"

using namespace EspINA;
using namespace EspINA::Query;

//------------------------------------------------------------------------
SampleSPtr EspINA::Query::sample(ChannelSPtr channel)
{
  return sample(channel.get());
}

//------------------------------------------------------------------------
SampleSPtr EspINA::Query::sample(ChannelPtr channel)
{
  SampleSPtr sample;

  auto relationships = channel->analysis()->relationships();
  auto samples = relationships->ancestors(channel, Channel::STAIN_LINK);

  if (!samples.isEmpty())
  {
    Q_ASSERT(samples.size() == 1); // Even with tiling, channels can only have 1 sample
    sample = std::dynamic_pointer_cast<Sample>(samples[0]);
  }

  return sample;
}

//------------------------------------------------------------------------
SampleSList EspINA::Query::samples(SegmentationSPtr segmentation)
{
  return samples(segmentation.get());
}

//------------------------------------------------------------------------
SampleSList EspINA::Query::samples(SegmentationPtr segmentation)
{
  SampleSList samples;

  auto relationships = segmentation->analysis()->relationships();
  auto relatedItems = relationships->ancestors(segmentation, Query::CONTAINS);

  for(auto item : relatedItems)
  {
    samples << std::dynamic_pointer_cast<Sample>(item);
  }

  return samples;
}

//------------------------------------------------------------------------
ChannelSList EspINA::Query::channels(SampleSPtr sample)
{
  ChannelSList channels;

  auto relationships = sample->analysis()->relationships();
  for(auto item : relationships->successors(sample, Channel::STAIN_LINK))
  {
    channels << std::dynamic_pointer_cast<Channel>(item);
  }

  return channels;
}

//------------------------------------------------------------------------
ChannelSList EspINA::Query::channels(SegmentationSPtr segmentation)
{
  ChannelSList channels;

  auto content = segmentation->analysis()->content();

  // Find first channel ancestors
  auto ancestors = content->ancestors(segmentation);

  while (!ancestors.isEmpty())
  {
    auto ancestor = ancestors.takeFirst();

    auto successors = content->successors(ancestor);

    ChannelSPtr channel;
    int i = 0;
    while (!channel && i < successors.size())
    {
      channel = std::dynamic_pointer_cast<Channel>(successors[i]);
      ++i;
    }

    if (channel && !channels.contains(channel))
    {
      channels << channel;
    } else
    {
      ancestors << content->ancestors(ancestor);
    }
  }

  return channels;
}


//------------------------------------------------------------------------
SegmentationSList EspINA::Query::segmentations(SampleSPtr sample)
{
  SegmentationSList segmentations;

  auto relationships = sample->analysis()->relationships();
  auto relatedItems = relationships->successors(sample, Query::CONTAINS);

  for(auto item : relatedItems)
  {
    segmentations << std::dynamic_pointer_cast<Segmentation>(item);
  }

  return segmentations;
}
