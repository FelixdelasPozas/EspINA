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

namespace EspINA
{
  class EspinaCore_EXPORT Analysis
  {
  public:
    explicit Analysis();

    void reset();

    void setClassification(ClassificationSPtr classification);

    ClassificationSPtr classification() const
    {return m_classification;}

    void add(SampleSPtr        sample);
    void add(SampleSList       samples);
    void add(ChannelSPtr       channel);
    void add(ChannelSList      channels);
    void add(SegmentationSPtr  segmentation);
    void add(SegmentationSList segmentations);

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

    void addRelation(AnalysisItemSPtr    ancestor,
                     AnalysisItemSPtr    succesor,
                     const RelationName& relation);

    void deleteRelation(AnalysisItemSPtr    ancestor,
                        AnalysisItemSPtr    succesor,
                        const RelationName& relation);

//     virtual ModelItemSList relatedItems(ModelItemPtr   item,
//                                         RelationType   relType,
//                                      const QString &relName = "");
//     virtual RelationList relations(ModelItemPtr   item,
//                                    const QString &relName = "");

    const DirectedGraphSPtr relationships()
    { return m_relations; }

    const DirectedGraphSPtr pipeline()
    { return m_pipeline; }

//     //---------------------------------------------------------------------------
//     /************************** SmartPointer API *******************************/
//     //---------------------------------------------------------------------------
//     AnalysisItemSPtr find(AnalysisItemPtr item);
// 
//     SampleSPtr findSample(AnalysisItemPtr item  );
//     SampleSPtr findSample(SamplePtr       sample);
// 
//     ChannelSPtr findChannel(AnalysisItemPtr item   );
//     ChannelSPtr findChannel(ChannelPtr      channel);
// 
//     SegmentationSPtr findSegmentation(AnalysisItemPtr item        );
//     SegmentationSPtr findSegmentation(SegmentationPtr segmentation);
// 
//     FilterSPtr findFilter(AnalysisItemPtr item  );
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
    void add(FilterSPtr  filter  );
    void add(FilterSList filters );

    void remove(FilterSPtr filter);

    void addClassification(CategorySPtr root);

    void addSampleImplementation   (SampleSPtr sample);
    void removeSampleImplementation(SampleSPtr sample);

    void addChannelImplementation   (ChannelSPtr channel);
    void removeChannelImplementation(ChannelSPtr channel);

    void addSegmentationImplementation   (SegmentationSPtr segmentation);
    void removeSegmentationImplementation(SegmentationSPtr segmentation);

    void addFilterImplementation   (FilterSPtr filter);
    void removeFilterImplementation(FilterSPtr filter);

    FilterSList filters() const
    { return m_filters; }


  private:
    ClassificationSPtr m_classification;
    DirectedGraphSPtr  m_relations;
    DirectedGraphSPtr  m_pipeline;

    ChannelSList      m_channels;
    FilterSList       m_filters;
    SampleSList       m_samples;
    SegmentationSList m_segmentations;
  };

  using AnalysisPtr  = Analysis *;
  using AnalysisSPtr = std::shared_ptr<Analysis>;
} // namespace EspINA

#endif // ESPINA_ANALYSIS_H
