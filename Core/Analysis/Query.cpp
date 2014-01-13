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

using namespace EspINA;
using namespace EspINA::Query;

//------------------------------------------------------------------------
SampleSPtr EspINA::Query::sample(SegmentationPtr segmentation)
{
}

//------------------------------------------------------------------------
SampleSPtr EspINA::Query::sample(SegmentationSPtr segmentation)
{
  return sample(segmentation.get());
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
