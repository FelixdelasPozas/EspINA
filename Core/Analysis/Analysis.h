/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Analysis/Graph/DirectedGraph.h"
#include "Category.h"
#include "ViewItem.h"

namespace EspINA
{
  class EspinaCore_EXPORT Analysis
  {
  public:
    struct Existing_Item_Exception{};
    struct Existing_Relation_Exception{};
    struct Item_Not_Found_Exception {};
    struct Relation_Not_Found_Exception {};

  public:
    explicit Analysis();

    void reset();

    void setStorage(TemporalStorageSPtr storage);

    TemporalStorageSPtr storage() const
    { return m_storage; }

    void setClassification(ClassificationSPtr classification);

    ClassificationSPtr classification() const
    {return m_classification;}

    void add(SampleSPtr           sample);
    void add(SampleSList          samples);
    void add(ChannelSPtr          channel);
    void add(ChannelSList         channels);
    void add(SegmentationSPtr     segmentation);
    void add(SegmentationSList    segmentations);

    void remove(SampleSPtr        sample);
    void remove(SampleSList       samples);
    void remove(ChannelSPtr       channel);
    void remove(ChannelSList      channels);
    void remove(SegmentationSPtr  segmentation);
    void remove(SegmentationSList segmentations);

    SampleSList samples() const
    { return m_samples; }

    ChannelSList channels() const
    { return m_channels; }

    SegmentationSList segmentations() const
    { return m_segmentations; }

    void addRelation(PersistentSPtr    ancestor,
                     PersistentSPtr    succesor,
                     const RelationName& relation);

    void deleteRelation(PersistentSPtr    ancestor,
                        PersistentSPtr    succesor,
                        const RelationName& relation);

//     virtual ModelItemSList relatedItems(ModelItemPtr   item,
//                                         RelationType   relType,
//                                      const QString &relName = "");
//     virtual RelationList relations(ModelItemPtr   item,
//                                    const QString &relName = "");
    const DirectedGraphSPtr relationships()
    { return m_relations; }

    const DirectedGraphSPtr content()
    { return m_content; }

  private:
    bool removeIfIsolated(DirectedGraphSPtr graph ,PersistentSPtr item);

    void addIfNotExists(FilterSPtr filter);

    void removeIfIsolated(FilterSPtr filter);

    void addFilterContentRelation(FilterSPtr filter, ViewItem* item);
    void addFilterContentRelation(FilterSPtr filter, ViewItemSPtr item);

    void removeFilterContentRelation(FilterSPtr filter, ViewItem* item);
    void removeFilterContentRelation(FilterSPtr filter, ViewItemSPtr item);

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
} // namespace EspINA

#endif // ESPINA_ANALYSIS_H
