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
#include "Query.h"
#include "Sample.h"
#include "Analysis.h"
#include "Channel.h"
#include "Segmentation.h"

// IMPORTANT NOTE: If current relation queries are not enough to retrieve segmentation channels
//                 we add extra check to search by content.

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
        auto analysis   = channel->analysis();
        auto relations  = analysis->relationships();
        auto sampleList = relations->ancestors(channel, Channel::STAIN_LINK);

        if (!sampleList.isEmpty())
        {
          Q_ASSERT(sampleList.size() == 1); // Even with tiling, channels can only have 1 sample
          sample = std::dynamic_pointer_cast<Sample>(sampleList.first());
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
      SampleSList samplesList;

      if (segmentation && segmentation->analysis())
      {
        auto analysis     = segmentation->analysis();
        auto relations    = analysis->relationships();
        auto relatedItems = relations->ancestors(segmentation, Sample::CONTAINS);

        auto castOperation = [&samplesList](const DirectedGraph::Vertex vertex) { auto sample = std::dynamic_pointer_cast<Sample>(vertex); if(sample) samplesList << sample; };
        std::for_each(relatedItems.constBegin(), relatedItems.constEnd(), castOperation);
      }

      return samplesList;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SampleSPtr sample)
    {
      ChannelSList stacks;

      if (sample && sample->analysis())
      {
        auto analysis  = sample->analysis();
        auto relations = analysis->relationships();
        auto relatedItems = relations->successors(sample, Channel::STAIN_LINK);

        auto castOperation = [&stacks] (const DirectedGraph::Vertex vertex) { auto stack = std::dynamic_pointer_cast<Channel>(vertex); if(stack) stacks << stack; };
        std::for_each(relatedItems.constBegin(), relatedItems.constEnd(), castOperation);
      }

      return stacks;
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
        auto analysisContents = segmentation->analysis()->content();

        // Find first channel ancestors
        auto ancestors = analysisContents->ancestors(segmentation);

        while (!ancestors.isEmpty())
        {
          auto ancestor = ancestors.takeFirst();
          auto successors = analysisContents->successors(ancestor);

          ChannelSPtr stack = nullptr;
          int i = 0;
          while (!stack && i < successors.size())
          {
            stack = std::dynamic_pointer_cast<Channel>(successors[i]);
            ++i;
          }

          if (stack)
          {
            if(!channels.contains(stack))
            {
              channels << stack;
            }
          }
          else
          {
            for(auto itemAncestor: analysisContents->ancestors(ancestor))
            {
              if(!ancestors.contains(itemAncestor))
              {
                ancestors << itemAncestor;
              }
            }
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
        auto analysis     = sample->analysis();
        auto relations    = analysis->relationships();
        auto relatedItems = relations->successors(sample, Sample::CONTAINS);

        auto castOperation = [&segmentations] (const DirectedGraph::Vertex vertex) { auto seg = std::dynamic_pointer_cast<Segmentation>(vertex); if(seg) segmentations << seg; };
        std::for_each(relatedItems.constBegin(), relatedItems.constEnd(), castOperation);
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
  } // namespace QueryContents

  namespace QueryRelations
  {
    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelSPtr channel)
    {
      auto analysis    = channel->analysis();
      auto samplesList = analysis->relationships()->ancestors(channel, Channel::STAIN_LINK);

      SampleSPtr sample{nullptr};

      if (samplesList.size() == 1)
      {
        sample = std::dynamic_pointer_cast<Sample>(samplesList.first());
      }
      else if (samplesList.size() > 1)
      {
        qWarning() << "Query Relations: Unexpected number of channel samples";
      }

      return sample;
    }

    //------------------------------------------------------------------------
    SampleSPtr sample(ChannelPtr channel)
    {
      auto stacks = channel->analysis()->channels();

      auto it = std::find_if(stacks.begin(), stacks.end(), [channel](const ChannelSPtr otherChannel) { return otherChannel.get() == channel; });

      if(it != stacks.end())
      {
        return sample(*it);
      }

      return SampleSPtr();
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationSPtr segmentation)
    {
      auto analysis   = segmentation->analysis();
      auto vertexList = analysis->relationships()->ancestors(segmentation, Sample::CONTAINS);

      SampleSList segmentationSamples;
      auto castOperation = [&segmentationSamples](const DirectedGraph::Vertex vertex) { auto pointer = std::dynamic_pointer_cast<Sample>(vertex); if(pointer) segmentationSamples << pointer; };
      std::for_each(vertexList.constBegin(), vertexList.constEnd(), castOperation);

      return segmentationSamples;
    }

    //------------------------------------------------------------------------
    SampleSList samples(SegmentationPtr segmentation)
    {
      const auto segmentations = segmentation->analysis()->segmentations();

      auto it = std::find_if(segmentations.constBegin(), segmentations.constEnd(), [segmentation](const SegmentationSPtr otherSeg) { return otherSeg.get() == segmentation; });

      if(it != segmentations.end())
      {
        return samples(*it);
      }

      return SampleSList();
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SampleSPtr sample)
    {
      const auto vertexList = sample->analysis()->relationships()->successors(sample, Channel::STAIN_LINK);

      ChannelSList stackList;
      std::for_each(vertexList.constBegin(), vertexList.constEnd(), [&stackList](const DirectedGraph::Vertex vertex) { auto stack = std::dynamic_pointer_cast<Channel>(vertex); if(stack) stackList << stack; });

      return stackList;
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationSPtr segmentation)
    {
      // NOTE: there is no way using relations to know what channel is the one
      // used as the origin for the segmentation. Using contents is the only way.
      return QueryContents::channels(segmentation);
    }

    //------------------------------------------------------------------------
    ChannelSList channels(SegmentationPtr segmentation)
    {
      auto segmentations = segmentation->analysis()->segmentations();

      auto it = std::find_if(segmentations.begin(), segmentations.end(), [segmentation](const SegmentationSPtr otherSeg) { return otherSeg.get() == segmentation; });

      if(it != segmentations.end())
      {
        return channels(*it);
      }

      return ChannelSList();
    }

    //------------------------------------------------------------------------
    SegmentationSList segmentations(SampleSPtr sample)
    {
      const auto segmentations = sample->analysis()->relationships()->successors(sample, Sample::CONTAINS);

      SegmentationSList segmentationList;
      std::for_each(segmentations.constBegin(), segmentations.constEnd(), [&segmentationList](const DirectedGraph::Vertex vertex) { auto segSPtr = std::dynamic_pointer_cast<Segmentation>(vertex); if(segSPtr) segmentationList << segSPtr; });

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
} // namespace ESPINA
