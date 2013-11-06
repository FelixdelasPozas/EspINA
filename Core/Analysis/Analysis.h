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

    void setClassification(ClassificationSPtr classification);

    ClassificationSPtr classification() const
    {return m_classification;}

    void add(SampleSPtr            sample);
    void add(SampleSList           samples);
    void add(ChannelSPtr           channel);
    void add(ChannelSList          channels);
    void add(SegmentationSPtr      segmentation);
    void add(SegmentationSList     segmentations);
    void add(ExtensionProviderSPtr provider);

    void remove(SampleSPtr            sample);
    void remove(SampleSList           samples);
    void remove(ChannelSPtr           channel);
    void remove(ChannelSList          channels);
    void remove(SegmentationSPtr      segmentation);
    void remove(SegmentationSList     segmentations);
    void remove(ExtensionProviderSPtr provider);

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
    ExtensionProviderSList extensionProviders() const
    { return m_providers; }

    const DirectedGraphSPtr relationships()
    { return m_relations; }

    const DirectedGraphSPtr content()
    { return m_content; }

//     //---------------------------------------------------------------------------
//     /************************** SmartPointer API *******************************/
//     //---------------------------------------------------------------------------
//     PersistentSPtr find(PersistentPtr item);
// 
//     SampleSPtr findSample(PersistentPtr item  );
//     SampleSPtr findSample(SamplePtr       sample);
// 
//     ChannelSPtr findChannel(PersistentPtr item   );
//     ChannelSPtr findChannel(ChannelPtr      channel);
// 
//     SegmentationSPtr findSegmentation(PersistentPtr item        );
//     SegmentationSPtr findSegmentation(SegmentationPtr segmentation);
// 
//     FilterSPtr findFilter(PersistentPtr item  );
//     FilterSPtr findFilter(FilterPtr       filter);

//     // signal emission methods, used by undo commands to signal finished operations.
//     void emitSegmentationAdded(SegmentationSList);
//     void emitChannelAdded(ChannelSList);
// 
//   signals:
//     void classificationAdded  (ClassificationSPtr classification);
//     void classificationRemoved(ClassificationSPtr classification);
// 
//     void sampleAdded  (SampleSPtr samples);
//     void sampleRemoved(SampleSPtr samples);
// 
//     void channelAdded  (ChannelSPtr channel);
//     void channelRemoved(ChannelSPtr channel);
// 
//     void segmentationAdded  (SegmentationSPtr segmentations);
//     void segmentationRemoved(SegmentationSPtr segmentations);
// 
//     void filterAdded  (FilterSPtr filter);
//     void filterRemoved(FilterSPtr filter);

  private:
    bool removeIfIsolated(DirectedGraphSPtr graph ,PersistentSPtr item);

    void addIfNotExists(FilterSPtr filter);
    void removeIfIsolated(FilterSPtr filter);

    bool findRelation(PersistentSPtr    ancestor,
                      PersistentSPtr    succesor,
                      const RelationName& relation);


  private:
    ClassificationSPtr m_classification;
    DirectedGraphSPtr  m_relations;
    DirectedGraphSPtr  m_content;

    ChannelSList           m_channels;
    FilterSList            m_filters; // NOTE: Could be removed
    SampleSList            m_samples;
    SegmentationSList      m_segmentations;
    ExtensionProviderSList m_providers;
  };

  using AnalysisPtr  = Analysis *;
  using AnalysisSPtr = std::shared_ptr<Analysis>;
} // namespace EspINA

#endif // ESPINA_ANALYSIS_H
