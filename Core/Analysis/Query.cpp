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

SampleSPtr EspINA::Query::sample(SegmentationSPtr segmentation)
{
  return sample(segmentation.get());
}

//------------------------------------------------------------------------
SampleSPtr EspINA::Query::sample(SegmentationPtr segmentation)
{
  auto segmentationSamples = samples(segmentation);
  Q_ASSERT(segmentationSamples.size() <= 1);

  return segmentationSamples.first();
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
ChannelSPtr EspINA::Query::channel(SegmentationSPtr segmentation)
{

}

//------------------------------------------------------------------------
ChannelSList EspINA::Query::channels(SampleSPtr sample)
{
  ChannelSList channels;

  auto relationships = sample->analysis()->relationships();
  for(auto item : relationships->succesors(sample, Channel::STAIN_LINK))
  {
    channels << std::dynamic_pointer_cast<Channel>(item);
  }

  return channels;
}

//------------------------------------------------------------------------
SegmentationSList EspINA::Query::segmentations(SampleSPtr sample)
{

}
