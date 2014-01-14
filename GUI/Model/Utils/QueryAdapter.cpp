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
  return ChannelAdapterSList();
}

//------------------------------------------------------------------------
SampleAdapterSList QueryAdapter::samples(ChannelAdapterSPtr segmentation)
{
  return samples(segmentation.get());
}

//------------------------------------------------------------------------
SampleAdapterSList QueryAdapter::samples(ChannelAdapterPtr segmentation)
{
  SampleAdapterSList channelSamples;

//   for(auto sample : Query::samples(segmentation))
//   {
//
//   }

  return channelSamples;
}

