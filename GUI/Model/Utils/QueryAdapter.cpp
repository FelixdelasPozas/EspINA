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

// ESPINA
#include "QueryAdapter.h"
#include <Core/Analysis/Query.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>

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
SegmentationAdapterSList QueryAdapter::segmentationsOnChannel(ChannelAdapterPtr channel)
{
  SegmentationAdapterSList result;

  auto segmentations = channel->model()->segmentations();
  for(auto segmentation: segmentations)
  {
    auto channels = QueryAdapter::channels(segmentation);

    for(auto item: channels)
    {
      if(item.get() == channel)
      {
        result << segmentation;
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::segmentationsOnChannel(ChannelAdapterSPtr channel)
{
  return segmentationsOnChannel(channel.get());
}

//------------------------------------------------------------------------
SampleAdapterSPtr QueryAdapter::smartPointer(ModelAdapterPtr model, SampleSPtr adaptedSample)
{
  const auto samples = model->samples();

  auto equalOp = [adaptedSample](const SampleAdapterSPtr sample) { return (sample->m_sample == adaptedSample); };
  auto it = std::find_if(samples.constBegin(), samples.constEnd(), equalOp);
  if(it != samples.constEnd())
  {
    return *it;
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
    auto equalOp = [channel](const ChannelSPtr adaptedStack) { return (channel->m_channel == adaptedStack); };
    auto it = std::find_if(adaptedChannels.constBegin(), adaptedChannels.constEnd(), equalOp);

    if(it != adaptedChannels.constEnd())
    {
      adaptedChannels.removeOne(*it);
      channels << channel;
    }

    if (adaptedChannels.isEmpty()) break;
  }

  return channels;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr QueryAdapter::smartPointer(ModelAdapterPtr model, SegmentationSPtr adaptedSegmentation)
{
  const auto segmentations = model->segmentations();

  auto selectionOp = [adaptedSegmentation](const SegmentationAdapterSPtr segmentation) { return (segmentation->m_segmentation == adaptedSegmentation); };
  auto it = std::find_if(segmentations.constBegin(), segmentations.constEnd(), selectionOp);
  if(it != segmentations.constEnd())
  {
    return *it;
  }

  return SegmentationAdapterSPtr();
}

//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::segmentationsOfCategory(ModelAdapterSPtr model, CategoryAdapterSPtr category)
{
  SegmentationAdapterSList results;

  auto selectionOp = [&results, category](const SegmentationAdapterSPtr segmentation)
  {
    if((category == segmentation->category()) || (segmentation->category()->classificationName().startsWith(category->classificationName(), Qt::CaseSensitive)))
    {
      results << segmentation;
    }
  };

  std::for_each(model->segmentations().constBegin(), model->segmentations().constEnd(), selectionOp);

  return results;
}

//------------------------------------------------------------------------
SegmentationAdapterSList QueryAdapter::smartPointer(ModelAdapterPtr model, SegmentationSList adaptedSegmentations)
{
  SegmentationAdapterSList segmentations;

  for (auto segmentation : model->segmentations())
  {
    auto equalOp = [segmentation](const SegmentationSPtr adaptedSegmentation) { return (segmentation->m_segmentation == adaptedSegmentation); };
    auto it = std::find_if(adaptedSegmentations.constBegin(), adaptedSegmentations.constEnd(), equalOp);
    if(it != adaptedSegmentations.constEnd())
    {
      adaptedSegmentations.removeOne(*it);
      segmentations << segmentation;
    }

    if (adaptedSegmentations.isEmpty()) break;
  }

  return segmentations;
}
