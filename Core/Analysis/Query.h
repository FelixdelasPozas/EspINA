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

#ifndef ESPINA_CORE_QUERY_H
#define ESPINA_CORE_QUERY_H

#include <Core/EspinaTypes.h>

namespace EspINA
{
  namespace QueryContents
  {
    SampleSPtr sample(ChannelSPtr channel);

    SampleSPtr sample(ChannelPtr channel);

    SampleSList samples(SegmentationSPtr segmentation);

    SampleSList samples(SegmentationPtr segmentation);

    ChannelSList channels(SampleSPtr sample);

    ChannelSList channels(SegmentationSPtr segmentation);

    ChannelSList channels(SegmentationPtr segmentation);

    SegmentationSList segmentations(SampleSPtr sample);

    SegmentationSList segmentationsOnChannelSample(ChannelSPtr channel);

    SegmentationSList segmentationsOnChannelSample(ChannelPtr channel);
  } // namespace QueryContents

  namespace QueryRelations
  {
    SampleSPtr sample(ChannelSPtr channel);

    SampleSPtr sample(ChannelPtr channel);

    SampleSList samples(SegmentationSPtr segmentation);

    SampleSList samples(SegmentationPtr segmentation);

    ChannelSList channels(SampleSPtr sample);

    ChannelSList channels(SegmentationSPtr segmentation);

    ChannelSList channels(SegmentationPtr segmentation);

    SegmentationSList segmentations(SampleSPtr sample);

    SegmentationSList segmentationsOnChannelSample(ChannelSPtr channel);

    SegmentationSList segmentationsOnChannelSample(ChannelPtr channel);
  }

} // namespace EspINA

#endif // ESPINA_CORE_QUERY_H
