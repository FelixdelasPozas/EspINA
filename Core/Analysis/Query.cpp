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

#include "Query.h"
#include "Sample.h"
#include "Analysis.h"
#include "Channel.h"
#include "Segmentation.h"

// IMPORTANT NOTE: If current relation queries are not enough to retrieve segmentation channels
//                 we add extra check to search by content

namespace ESPINA
{
  namespace QueryContents
  {
    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelSPtr channel)
    {
      return sample(channel.get());
    }

    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelPtr channel)
    {
      SampleSPtr sample;

      if (channel && channel->analysis())
      {
        auto analysis      = channel->analysis();
        auto relationships = analysis->relationships();
        auto samples       = relationships->ancestors(channel, Channel::STAIN_LINK);

        if (!samples.isEmpty())
        {
          Q_ASSERT(samples.size() == 1); // Even with tiling, channels can only have 1 sample
          sample = std::dynamic_pointer_cast<Sample>(samples[0]);
        }
      }

      return sample;
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationSPtr segmentation)
    {
      return samples(segmentation.get());
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationPtr segmentation)
    {
      SampleSList samples;

      if (segmentation && segmentation->analysis())
      {
        auto analysis      = segmentation->analysis();
        auto relationships = analysis->relationships();
        auto relatedItems  = relationships->ancestors(segmentation, Sample::CONTAINS);

        for(auto item : relatedItems)
        {
          samples << std::dynamic_pointer_cast<Sample>(item);
        }
      }

      return samples;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SampleSPtr sample)
    {
      ChannelSList channels;

      if (sample && sample->analysis())
      {
        auto analysis      = sample->analysis();
        auto relationships = analysis->relationships();

        for(auto item : relationships->successors(sample, Channel::STAIN_LINK))
        {
          channels << std::dynamic_pointer_cast<Channel>(item);
        }
      }

      return channels;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationSPtr segmentation)
    {
      return channels(segmentation.get());
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationPtr segmentation)
    {
      ChannelSList channels;

      if (segmentation && segmentation->analysis())
      {
        auto analysis = segmentation->analysis();
        auto content  = analysis->content();

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
      }

      return channels;
    }


    //------------------------------------------------------------------------
    SegmentationSList segmentations(SampleSPtr sample)
    {
      SegmentationSList segmentations;

      if (sample && sample->analysis())
      {
        auto analysis      = sample->analysis();
        auto relationships = analysis->relationships();
        auto relatedItems  = relationships->successors(sample, Sample::CONTAINS);

        for(auto item : relatedItems)
        {
          segmentations << std::dynamic_pointer_cast<Segmentation>(item);
        }
      }

      return segmentations;
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentationsOnChannelSample(ChannelSPtr channel)
    {
      return segmentationsOnChannelSample(channel.get());
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentationsOnChannelSample(ChannelPtr channel)
    {
      return segmentations(sample(channel));
    }

  }

  namespace QueryRelations
  {
    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelSPtr channel)
    {
      auto analysis = channel->analysis();
      auto samples  = analysis->relationships()->ancestors(channel, Channel::STAIN_LINK);

      SampleSPtr sample;

      if (samples.size() == 1)
      {
        sample = std::dynamic_pointer_cast<Sample>(samples.first());
      }
      else if (samples.size() > 1)
      {
        qWarning() << "Query Relations: Unexpected number of channel samples";
      }

      return sample;
    }

    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelPtr channel)
    {
      auto channels = channel->analysis()->channels();

      for(auto analysisChannel: channels)
      {
        if(analysisChannel.get() == channel)
          return sample(analysisChannel);
      }

      return SampleSPtr();
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationSPtr segmentation)
    {
      auto analysis = segmentation->analysis();
      auto samples = analysis->relationships()->ancestors(segmentation, Sample::CONTAINS);

      SampleSList samplesList;
      for(auto sample : samples)
      {
        samplesList << std::dynamic_pointer_cast<Sample>(sample);
      }

      return samplesList;
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationPtr segmentation)
    {
      auto segmentations = segmentation->analysis()->segmentations();

      for(auto analysisSegmentation: segmentations)
      {
        if(analysisSegmentation.get() == segmentation)
          return samples(analysisSegmentation);
      }

      SampleSList emptyList;
      return emptyList;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SampleSPtr sample)
    {
      auto channels = sample->analysis()->relationships()->successors(sample, Channel::STAIN_LINK);

      ChannelSList channelsList;
      for (auto channel : channels)
      {
        auto channelPointer = std::dynamic_pointer_cast<Channel>(channel);
        Q_ASSERT(channelPointer != nullptr);
        channelsList << channelPointer;
      }

      return channelsList;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationSPtr segmentation)
    {
      auto samplesSList = samples(segmentation);

      ChannelSList channelList;
      for (auto sample : samplesSList)
        channelList << channels(sample);

      // In case no stain relation is found we try to find using content
      if (channelList.isEmpty())
      {
        channelList = QueryContents::channels(segmentation);
      }

      return channelList;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationPtr segmentation)
    {
      auto segmentations = segmentation->analysis()->segmentations();

      for(auto analysisSegmentation: segmentations)
      {
        if(analysisSegmentation.get() == segmentation)
          return channels(analysisSegmentation);
      }

      return ChannelSList();
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentations(SampleSPtr sample)
    {
      auto segmentations = sample->analysis()->relationships()->successors(sample, Sample::CONTAINS);

      SegmentationSList segmentationList;
      for(auto segmentation: segmentations)
      {
        auto segmentationSPointer = std::dynamic_pointer_cast<Segmentation>(segmentation);
        Q_ASSERT(segmentationSPointer != nullptr);
        segmentationList << segmentationSPointer;
      }

      return segmentationList;
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentationsOnChannelSample(ChannelSPtr channel)
    {
      return segmentationsOnChannelSample(channel.get());
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentationsOnChannelSample(ChannelPtr channel)
    {
      auto channelSample = sample(channel);

      return segmentations(channelSample);
    }
  } // namespace QueryRelations

}
