/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ESPINA_ANALYSIS_H
#define ESPINA_ANALYSIS_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Types.h"
#include "Core/Analysis/Graph/DirectedGraph.h"
#include "Category.h"
#include "ViewItem.h"

namespace ESPINA
{
  class EspinaCore_EXPORT Analysis
  {
  public:
    struct Existing_Item_Exception{};
    struct Existing_Relation_Exception{};
    struct Item_Not_Found_Exception {};
    struct Relation_Not_Found_Exception {};

  public:
    /** \brief Analysis class constructor.
     *
     */
    explicit Analysis();

    ~Analysis();

    /** \brief Empties the analysis.
     *
     */
    void clear();

    /** \brief Sets the storage for the analysis.
     * \param[in] storage temporal storage object smart pointer.
     *
     */
    void setStorage(TemporalStorageSPtr storage);

    /** \brief Returns the temporal storage used by the analysis.
     *
     */
    TemporalStorageSPtr storage() const
    { return m_storage; }

    /** \brief Sets the classification for the analysis.
     * \param[in] classification classification object smart pointer.
     *
     */
    void setClassification(ClassificationSPtr classification);

    /** \brief Returns the classifiacation smart pointer.
     *
     */
    ClassificationSPtr classification() const
    {return m_classification;}

    /** \brief Adds a sample to the analysis.
     * \param[in] sample sample smart pointer.
     *
     */
    void add(SampleSPtr sample);

    /** \brief Adds a list of samples to the analysis.
     * \param[in] samples list of sample smart pointers.
     *
     */
    void add(SampleSList samples);

    /** \brief Adds a channel to the analysis.
     * \param[in] channel channel smart pointer.
     *
     */
    void add(ChannelSPtr channel);

    /** \brief Adds a list of channels to the analysis.
     * \param[in] channels list of channel smart pointers.
     *
     */
    void add(ChannelSList channels);

    /** \brief Adds a segmentation to the analysis.
     * \param[in] segmentation segmentation smart pointer.
     *
     */
    void add(SegmentationSPtr segmentation);

    /** \brief Adds a list of segmentations to the analysis.
     * \param[in] segmentations list of segmentation smart pointers.
     *
     */
    void add(SegmentationSList segmentations);

    /** \brief Removes a sample from the analysis.
     * \param[in] sample sample smart pointer.
     *
     */
    void remove(SampleSPtr sample);

    /** \brief Removes a list of samples from the analysis.
     * \param[in] samples list of sample smart pointers.
     *
     */
    void remove(SampleSList samples);

    /** \brief Removes a channel from the analysis.
     * \param[in] channel channel smart pointer.
     *
     */
    void remove(ChannelSPtr channel);

    /** \brief Removes a list of channels from the analysis.
     * \param[in] channels list of channel smart pointers.
     *
     */
    void remove(ChannelSList channels);

    /** \brief Removes a segmentation from the analysis.
     * \param[in] segmentation segmentation smart pointer.
     *
     */
    void remove(SegmentationSPtr segmentation);

    /** \brief Removes a list of segmentations from the analysis.
     * \param[in] segmentations list of segmentation smart pointers.
     *
     */
    void remove(SegmentationSList segmentations);

    void changeSpacing(ChannelSPtr channel, const NmVector3 &spacing);

    /** \brief Returns the list of samples of the analysis.
     *
     */
    SampleSList samples() const
    { return m_samples; }

    /** \brief Returns the list of channels of the analysis.
     *
     */
    ChannelSList channels() const
    { return m_channels; }

    /** \brief Returns the list of segmentations of the analysis.
     *
     */
    SegmentationSList segmentations() const
    { return m_segmentations; }

    /** \brief Adds a relation between to Persistent objects in the analysis.
     * \param[in] ancestor Persistent object smart pointer, origin of the relation.
     * \param[in] successor Persistent object smart pointer, destination of the relation.
     * \param[in] relation relation key.
     *
     */
    void addRelation(PersistentSPtr    ancestor,
                     PersistentSPtr    succesor,
                     const RelationName& relation);

    /** \brief Removes a relation between to Persistent objects in the analysis.
     * \param[in] ancestor Persistent object smart pointer, origin of the relation.
     * \param[in] successor Persistent object smart pointer, destination of the relation.
     * \param[in] relation relation key.
     *
     */
    void deleteRelation(PersistentSPtr    ancestor,
                        PersistentSPtr    succesor,
                        const RelationName& relation);

    /** \brief Return the relations graph of the analysis.
     * The relationship graph expresses the concept relations between persistent objects in the analysis.
     *
     */
    const DirectedGraphSPtr relationships()
    { return m_relations; }

    /** \brief Returns the content graph of the analysis.
     * The content graph expresses dependencies between persistent objects in the analysis.
     *
     */
    const DirectedGraphSPtr content()
    { return m_content; }

  private:
    /** \brief Removes a filter of the analysis if its isolated.
     * \param[in] filter smart pointer of the filter to check.
     *
     */
    void removeIfIsolated(FilterSPtr filter);

    /** \brief Removes a item (node) of the graph if its isolated (has no relations).
     * \param[in] graph directed graph to check for node.
     * \param[in] item item to check.
     *
     */
    bool removeIfIsolated(DirectedGraphSPtr graph ,PersistentSPtr item);

    /** \brief Adds a filter to the analysis only if it doesn't exist in the analysis.
     * \param[in] filter smart pointer of the filter to check.
     *
     */
    void addIfNotExists(FilterSPtr filter);

    /** \brief Returns all filters which form the pipeline from filter
     * \param[in] filter start of the pipeline sequence
     *
     */
    FilterSList downStreamPipeline(FilterSPtr filter);

    /** \brief Adds a relation in the content graph from the filter to the item.
     * \param[in] filter filter smart pointer.
     * \param[in] item view item raw pointer.
     *
     */
    void addFilterContentRelation(FilterSPtr filter, ViewItem* item);

    /** \brief Adds a relation in the content graph from the filter to the item.
     * \param[in] filter filter smart pointer.
     * \param[in] item view item smart pointer.
     *
     */
    void addFilterContentRelation(FilterSPtr filter, ViewItemSPtr item);

    /** \brief Removes a relation in the content graph from the filter to the item.
     * \param[in] filter filter smart pointer.
     * \param[in] item view item raw pointer.
     *
     */
    void removeFilterContentRelation(FilterSPtr filter, ViewItem* item);

    /** \brief Removes a relation in the content graph from the filter to the item.
     * \param[in] filter filter smart pointer.
     * \param[in] item view item smart pointer.
     *
     */
    void removeFilterContentRelation(FilterSPtr filter, ViewItemSPtr item);

    /** \brief Returns true if the specified relation exists in the analysis.
     * \param[in] ancestor smart pointer of the persistent object origin of the relation.
     * \param[in] successor smart pointer of the persistent object destination of the relation.
     * \param[in] relation relation key.
     *
     */
    bool findRelation(PersistentSPtr      ancestor,
                      PersistentSPtr      succesor,
                      const RelationName& relation);

  private:
    ClassificationSPtr  m_classification;
    DirectedGraphSPtr   m_relations;
    DirectedGraphSPtr   m_content;

    ChannelSList        m_channels;
    FilterSList         m_filters; // NOTE: Could be removed
    SampleSList         m_samples;
    SegmentationSList   m_segmentations;

    TemporalStorageSPtr m_storage;

    friend class ViewItem;
  };

  using AnalysisPtr  = Analysis *;
  using AnalysisSPtr = std::shared_ptr<Analysis>;

} // namespace ESPINA

#endif // ESPINA_ANALYSIS_H
