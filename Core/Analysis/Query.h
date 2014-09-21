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

#ifndef ESPINA_CORE_QUERY_H
#define ESPINA_CORE_QUERY_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>

namespace ESPINA
{
	/** brief Retrieve information contained in the content graph of the analysis.
	 *
	 */
  namespace QueryContents
  {
  	/** brief Returns the sample associated with the specified channel.
  	 * \param[in] channel, channel object smart pointer.
  	 *
  	 */
    SampleSPtr EspinaCore_EXPORT sample(ChannelSPtr channel);

  	/** brief Returns the sample associated with the specified channel.
  	 * \param[in] channel, channel object raw pointer.
  	 *
  	 */
    SampleSPtr EspinaCore_EXPORT sample(ChannelPtr channel);

  	/** brief Returns the list of samples associated with the specified segmentation.
  	 * \param[in] segmentation, segmentation object smart pointer.
  	 *
  	 */
    SampleSList EspinaCore_EXPORT samples(SegmentationSPtr segmentation);

  	/** brief Returns the list of samples associated with the specified segmentation.
  	 * \param[in] segmentation, segmentation object raw pointer.
  	 *
  	 */
    SampleSList EspinaCore_EXPORT samples(SegmentationPtr segmentation);

  	/** brief Returns the list of channels associated with a sample.
  	 * \param[in] sample, sample object smart pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SampleSPtr sample);

  	/** brief Returns the list of channels associated with a segmentation.
  	 * \param[in] segmentation, segmentation object smart pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SegmentationSPtr segmentation);

  	/** brief Returns the list of channels associated with a segmentation.
  	 * \param[in] segmentation, segmentation object raw pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SegmentationPtr segmentation);

  	/** brief Returns the list of segmentations associated with a sample.
  	 * \param[in] sample, sample object smart pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentations(SampleSPtr sample);

  	/** brief Returns the list of segmentations associated with a channel.
  	 * \param[in] channel, channel object smart pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentationsOnChannelSample(ChannelSPtr channel);

  	/** brief Returns the list of segmentations associated with a channel.
  	 * \param[in] channel, channel object raw pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentationsOnChannelSample(ChannelPtr channel);
  } // namespace QueryContents

	/** brief Retrieve information contained in the relations graph of the analysis.
	 *
	 */
  namespace QueryRelations
  {
  	/** brief Returns the sample associated with a channel.
  	 * \param[in] channel, channel object smart pointer.
  	 *
  	 */
  	SampleSPtr EspinaCore_EXPORT sample(ChannelSPtr channel);

  	/** brief Returns the sample associated with a channel.
  	 * \param[in] channel, channel object raw pointer.
  	 *
  	 */
    SampleSPtr EspinaCore_EXPORT sample(ChannelPtr channel);

  	/** brief Returns the list of samples associated with a segmentation.
  	 * \param[in] segmentation, segmentation object smart pointer.
  	 *
  	 */
    SampleSList EspinaCore_EXPORT samples(SegmentationSPtr segmentation);

  	/** brief Returns the list of samples associated with a segmentation.
  	 * \param[in] segmentation, segmentation object raw pointer.
  	 *
  	 */
    SampleSList EspinaCore_EXPORT samples(SegmentationPtr segmentation);

  	/** brief Returns the list of channels associated with a sample.
  	 * \param[in] sample, sample object smart pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SampleSPtr sample);

  	/** brief Returns the list of channels associated with a segmentation.
  	 * \param[in] segmentation, segmentation object smart pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SegmentationSPtr segmentation);

  	/** brief Returns the list of channels associated with a segmentation.
  	 * \param[in] segmentation, segmentation object raw pointer.
  	 *
  	 */
    ChannelSList EspinaCore_EXPORT channels(SegmentationPtr segmentation);

  	/** brief Returns the list of segmentations associated with a sample.
  	 * \param[in] sample, sample object smart pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentations(SampleSPtr sample);

  	/** brief Returns the list of segmentations associated with a channel.
  	 * \param[in] channel, channel object smart pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentationsOnChannelSample(ChannelSPtr channel);

  	/** brief Returns the list of segmentations associated with a channel.
  	 * \param[in] channel, channel object raw pointer.
  	 *
  	 */
    SegmentationSList EspinaCore_EXPORT segmentationsOnChannelSample(ChannelPtr channel);
  }

} // namespace ESPINA

#endif // ESPINA_CORE_QUERY_H
