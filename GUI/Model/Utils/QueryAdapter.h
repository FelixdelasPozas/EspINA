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

#ifndef ESPINA_QUERY_ADAPTER_H
#define ESPINA_QUERY_ADAPTER_H

#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/SampleAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>

namespace EspINA {

  class QueryAdapter
  {
  public:
    static ChannelAdapterSList channels(SegmentationAdapterPtr segmentation);

    static ChannelAdapterSList channels(SegmentationAdapterSPtr segmentation);

    static SampleAdapterSPtr sample(ChannelAdapterSPtr channel);

    static SampleAdapterSPtr sample(ChannelAdapterPtr channel);

    static SegmentationAdapterSList segmentationsOnChannelSample(ChannelAdapterSPtr channel);

    static SegmentationAdapterSList segmentationsOnChannelSample(ChannelAdapterPtr channel);

  private:
    static SampleAdapterSPtr        smartPointer(ModelAdapterPtr model, SampleSPtr adaptedSample);

    static ChannelAdapterSList      smartPointer(ModelAdapterPtr model, ChannelSList adaptedChannels);

    static SegmentationAdapterSPtr  smartPointer(ModelAdapterPtr model, SegmentationSPtr adaptedSegmentation);

    static SegmentationAdapterSList smartPointer(ModelAdapterPtr model, SegmentationSList adaptedSegmentations);
  };
} // namespace EspINA

#endif // ESPINA_QUERY_ADAPTER_H