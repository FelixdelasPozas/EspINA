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

#ifndef ESPINA_QUERY_ADAPTER_H
#define ESPINA_QUERY_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/SampleAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT QueryAdapter
  {
  public:
    /** \brief Returns the list of channel adapters related to a sample adapter.
     * \param[in] sample sample adapter raw pointer.
     *
     */
    static ChannelAdapterSList channels(SampleAdapterPtr sample);

    /** \brief Returns the list of channel adapters related to a sample adapter.
     * \param[in] sample sample adapter smart pointer.
     *
     */
    static ChannelAdapterSList channels(SampleAdapterSPtr sample);

    /** \brief Returns the list of channel adapters related to a segmentation adapter.
     * \param[in] segmentation segmentation adapter raw pointer.
     *
     */
    static ChannelAdapterSList channels(SegmentationAdapterPtr segmentation);

    /** \brief Returns the list of channel adapters related to a segmentation adapter.
     * \param[in] segmentation segmentation adapter smart pointer.
     *
     */
    static ChannelAdapterSList channels(SegmentationAdapterSPtr segmentation);

    /** \brief Returns the sample adapter related to a channel adapter.
     * \param[in] channel channel adapter smart pointer.
     *
     */
    static SampleAdapterSPtr sample(ChannelAdapterSPtr channel);

    /** \brief Returns the sample adapter related to a channel adapter.
     * \param[in] channel channel adapter raw pointer.
     *
     */
    static SampleAdapterSPtr sample(ChannelAdapterPtr channel);

    /** \brief Returns the sample adapter related to a segmentation adapter.
     * \param[in] segmentation segmentation adapter smart pointer.
     *
     */
    static SampleAdapterSList samples(SegmentationAdapterSPtr segmentation);

    /** \brief Returns the sample adapter related to a segmentation adapter.
     * \param[in] segmentation segmentation adapter raw pointer.
     *
     */
    static SampleAdapterSList samples(SegmentationAdapterPtr segmentation);

    /** \brief Returns the list of segmentation adapters related to a channel adapter.
     * \param[in] channel channel adapter smart pointer.
     *
     */
    static SegmentationAdapterSList segmentationsOnChannelSample(ChannelAdapterSPtr channel);

    /** \brief Returns the list of segmentation adapters related to a channel adapter.
     * \param[in] channel channel adapter raw pointer.
     *
     */
    static SegmentationAdapterSList segmentationsOnChannelSample(ChannelAdapterPtr channel);

  private:
    /** \brief Returns the sample adapter smart pointer of a sample contained in the specified model.
     * \param[in] model model adapter raw pointer.
     * \param[in] adaptedSample sample smart pointer.
     *
     */
    static SampleAdapterSPtr smartPointer(ModelAdapterPtr model, SampleSPtr adaptedSample);

    /** \brief Returns the list of sample adapter smart pointers of a list of samples contained in the specified model.
     * \param[in] model model adapter raw pointer.
     * \param[in] adaptedSamples list of sample smart pointers.
     *
     */
    static SampleAdapterSList smartPointer(ModelAdapterPtr model, SampleSList adaptedSamples);

    /** \brief Returns the list of channel adapter smart pointers of a list of channels contained in the specified model.
     * \param[in] model model adapter raw pointer.
     * \param[in] adaptedChannels list of channel smart pointers.
     *
     */
    static ChannelAdapterSList smartPointer(ModelAdapterPtr model, ChannelSList adaptedChannels);

    /** \brief Returns the segmentation adapter smart pointer of a segmentation contained in the specified model.
     * \param[in] model model adapter raw pointer.
     * \param[in] adaptedSegmentation segmentation adapter smart pointer.
     *
     */
    static SegmentationAdapterSPtr smartPointer(ModelAdapterPtr model, SegmentationSPtr adaptedSegmentation);

    /** \brief Returns the list of segmentation adapter smart pointers of a list of segmentations contained in the specified model.
     * \param[in] model model adapter raw pointer.
     * \param[in] adaptedSegmentations list of segmentation smart pointers.
     *
     */
    static SegmentationAdapterSList smartPointer(ModelAdapterPtr model, SegmentationSList adaptedSegmentations);
  };
} // namespace ESPINA

#endif // ESPINA_QUERY_ADAPTER_H
