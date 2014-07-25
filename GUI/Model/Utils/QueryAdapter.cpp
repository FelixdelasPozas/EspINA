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

#include "QueryAdapter.h"

#include <Core/Analysis/Query.h>
#include <GUI/Model/ModelAdapter.h>

using namespace ESPINA;

//------------------------------------------------------------------------
ChannelAdapterSList QueryAdapter::channels(SampleAdapterSPtr sample)
{
  return channels(sample.get());
}

//------------------------------------------------------------------------
ChannelAdapterSList QueryAdapter::channels(SampleAdapterPtr sample)
{
  auto adaptedSample = sample->m_sample;
  auto model = sample->model();

  ChannelAdapterSList adaptedChannels;

  return smartPointer(model, QueryRelations::channels(adaptedSample));
}

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

  return smartPointer(model, QueryRelations::channels(adaptedSegmentation));
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

  auto adaptedSample = QueryRelations::sample(adaptedChannel);

  return smartPointer(model, adaptedSample);
}

//------------------------------------------------------------------------
SampleAdapterSList QueryAdapter::samples(SegmentationAdapterSPtr segmentation)
{
  return samples(segmentation.get());
}

//------------------------------------------------------------------------
SampleAdapterSList QueryAdapter::samples(SegmentationAdapterPtr segmentation)
{
  auto adaptedSegmentation = segmentation->m_segmentation;
  auto model = segmentation->model();

  return smartPointer(model, QueryRelations::samples(adaptedSegmentation));
}

//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::segmentationsOnChannelSample(ChannelAdapterSPtr channel)
{
  return segmentationsOnChannelSample(channel.get());
}


//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::segmentationsOnChannelSample(ChannelAdapterPtr channel)
{
  auto adaptedChannel = channel->m_channel;
  auto model = channel->model();

  return smartPointer(model, QueryRelations::segmentationsOnChannelSample(adaptedChannel));
}


//------------------------------------------------------------------------
SampleAdapterSPtr QueryAdapter::smartPointer(ModelAdapterPtr model, SampleSPtr adaptedSample)
{
  for (auto sample : model->samples())
  {
    if (sample->m_sample == adaptedSample)
    {
      return sample;
    }
  }

  return SampleAdapterSPtr();
}

//------------------------------------------------------------------------
SampleAdapterSList QueryAdapter::smartPointer(ModelAdapterPtr model, SampleSList adaptedSamples)
{
  SampleAdapterSList samples;

  for (auto adaptedSample : adaptedSamples)
  {
    samples << smartPointer(model, adaptedSample);
  }

  return samples;
}

//------------------------------------------------------------------------
ChannelAdapterSList QueryAdapter::smartPointer(ModelAdapterPtr model, ChannelSList adaptedChannels)
{
  ChannelAdapterSList channels;

  for (auto channel : model->channels())
  {
    for (auto adaptedChannel : adaptedChannels)
    {
      if (channel->m_channel == adaptedChannel)
      {
        adaptedChannels.removeOne(adaptedChannel);
        channels << channel;
        break;
      }
    }

    if (adaptedChannels.isEmpty()) break;
  }

  return channels;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr QueryAdapter::smartPointer(ModelAdapterPtr model, SegmentationSPtr adaptedSegmentation)
{
  for (auto segmentation : model->segmentations())
  {
    if (segmentation->m_segmentation == adaptedSegmentation)
    {
      return segmentation;
    }
  }

  return SegmentationAdapterSPtr();
}

//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::smartPointer(ModelAdapterPtr model, SegmentationSList adaptedSegmentations)
{
  SegmentationAdapterSList segmentations;

  for (auto segmentation : model->segmentations())
  {
    for (auto adaptedSegmentation : adaptedSegmentations)
    {
      if (segmentation->m_segmentation == adaptedSegmentation)
      {
        adaptedSegmentations.removeOne(adaptedSegmentation);
        segmentations << segmentation;
        break;
      }
    }

    if (adaptedSegmentations.isEmpty()) break;
  }

  return segmentations;
}
