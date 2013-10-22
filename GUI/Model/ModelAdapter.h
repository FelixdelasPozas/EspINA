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

#ifndef ESPINA_MODEL_ADAPTER_H
#define ESPINA_MODEL_ADAPTER_H

#include "EspinaGUI_Export.h"

#include "GUI/Model/SampleAdapter.h"

#include <QAbstractItemModel>

namespace EspINA
{
  struct Relation
  {
    ItemAdapterSPtr ancestor;
    ItemAdapterSPtr succesor;
    RelationName    relation;
  };
  typedef QList<Relation> RelationList;

  class Analysis;
  using AnalysisSPtr = std::shared_ptr<Analysis>;


  /**
  *
  * Model elements are arranged in the following way:
  * QModelIndex() (invalid index/model root index)
  * - ClassificationRoot
  *   - Category 1
  *     - Sub-Catagory 1-1
  *     - ...
  *   - Category 2
  *     - ...
  *   - ...
  * - SampleAdapterRoot
  *   - SampleAdapter1
  *   - ...
  * - ChannelRoot
  *   - Channel1
  *   - ...
  * - SegmentationRoot
  *   - Segmentation1
  *   - ...
  * - FilterRoot
  *   - Filter1
  *   - ...
  */
  class EspinaGUI_EXPORT ModelAdapter
  : public QAbstractItemModel
  {
    Q_OBJECT
  public:
    explicit ModelAdapter(AnalysisSPtr analysis);
    virtual ~ModelAdapter();

    void setAnalysis(AnalysisSPtr analysis);

    void reset();

    // Implement QAbstractItemModel Interface
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    QModelIndex index(ItemAdapterPtr item) const;
    QModelIndex index(ItemAdapterSPtr item) const;

    // Special Nodes of the model to refer different roots
    QModelIndex classificationRoot() const;
    QModelIndex categoryIndex(CategoryPtr  category) const;
    QModelIndex categoryIndex(CategorySPtr category) const;

    QModelIndex sampleRoot() const;
    QModelIndex sampleIndex(SampleItemPtr  sample) const;
    QModelIndex sampleIndex(SampleAdapterSPtr sample) const;

    QModelIndex channelRoot() const;
    QModelIndex channelIndex(ChannelPtr  channel) const;
    QModelIndex channelIndex(ChannelSPtr channel) const;

    QModelIndex segmentationRoot() const;
    QModelIndex segmentationIndex(SegmentationPtr  segmentation) const;
    QModelIndex segmentationIndex(SegmentationSPtr segmentation) const;

    // Classification
    void setClassification(ClassificationSPtr classification);

    const ClassificationSPtr classification() const;

    CategorySPtr createCategory(const QString& name, CategoryPtr  parent=nullptr);
    CategorySPtr createCategory(const QString& name, CategorySPtr parent=CategorySPtr());

    void addCategory   (CategorySPtr category, CategorySPtr parent);
    void removeCategory(CategorySPtr category, CategorySPtr parent);

    //TODO 2013-10-21: Throw exception if they don't belong to the same classification
    void reparentCategory(CategorySPtr category, CategorySPtr parent);

    void add(SampleAdapterSPtr        sample);
    void add(SampleAdapterSList       samples);
    void add(ChannelSPtr       channel);
    void add(ChannelSList      channels);
    void add(SegmentationSPtr  segmentation);
    void add(SegmentationSList segmentations);

    void remove(SampleAdapterSPtr        sample);
    void remove(SampleAdapterSList       samples);
    void remove(ChannelSPtr       channel);
    void remove(ChannelSList      channels);
    void remove(SegmentationSPtr  segmentation);
    void remove(SegmentationSList segmentations);

    SampleAdapterSList samples() const
    { return m_samples; }

    ChannelSList channels() const
    { return m_channels; }

    SegmentationSList segmentations() const
    { return m_segmentations; }

    void setSegmentationCategory(SegmentationSPtr segmentation,
                                 CategorySPtr     category);


    void addRelation(ItemAdapterSPtr     ancestor,
                     ItemAdapterSPtr     succesor,
                     const RelationName& relation);

    void deleteRelation(ItemAdapterSPtr     ancestor,
                        ItemAdapterSPtr     succesor,
                        const RelationName& relation);

    ItemAdapterSList relatedItems(ItemAdapterSPtr    item,
                                  RelationType        type,
                                  const RelationName& filter = "");

    RelationList relations(ItemAdapterSPtr     item,
                           RelationType        type,
                           const RelationName& filter = "");


//     //---------------------------------------------------------------------------
//     /************************** SmartPointer API *******************************/
//     //---------------------------------------------------------------------------
//     virtual ModelItemSPtr find(ModelItemPtr item);
// 
//     virtual CategorySPtr findCategory(ModelItemPtr       item           );
//     virtual CategorySPtr findCategory(CategoryPtr taxonomyElement);
// 
//     virtual SampleAdapterSPtr findSampleAdapter(ModelItemPtr item  );
//     virtual SampleAdapterSPtr findSampleAdapter(SampleAdapterPtr    sample);
// 
//     virtual ChannelSPtr findChannel(ModelItemPtr item   );
//     virtual ChannelSPtr findChannel(ChannelPtr   channel);
// 
//     virtual SegmentationSPtr findSegmentation(ModelItemPtr    item        );
//     virtual SegmentationSPtr findSegmentation(SegmentationPtr segmentation);
// 
//     virtual FilterSPtr findFilter(ModelItemPtr item  );
//     virtual FilterSPtr findFilter(FilterPtr    filter);

    // signal emission methods, used by undo commands to signal finished operations.
    void emitSegmentationAdded(SegmentationSList);
    void emitChannelAdded(ChannelSList);

  signals:
    void taxonomyAdded  (ClassificationSPtr taxonomy);
    void taxonomyRemoved(ClassificationSPtr taxonomy);

    void sampleAdded  (SampleAdapterSPtr samples);
    void sampleRemoved(SampleAdapterSPtr samples);

    void channelAdded  (ChannelSPtr channel);
    void channelRemoved(ChannelSPtr channel);

    void segmentationAdded  (SegmentationSPtr segmentations);
    void segmentationRemoved(SegmentationSPtr segmentations);

    void filterAdded  (FilterSPtr filter);
    void filterRemoved(FilterSPtr filter);

  private slots:
    void itemModified(ModelItemPtr item);

  private:
    void addClassification(CategorySPtr root);

    void addSampleAdapterImplementation   (SampleAdapterSPtr sample);
    void removeSampleAdapterImplementation(SampleAdapterSPtr sample);

    void addChannelImplementation   (ChannelSPtr channel);
    void removeChannelImplementation(ChannelSPtr channel);

    void addSegmentationImplementation   (SegmentationSPtr segmentation);
    void removeSegmentationImplementation(SegmentationSPtr segmentation);

    void addFilterImplementation   (FilterSPtr filter);
    void removeFilterImplementation(FilterSPtr filter);

  private:
    EspinaFactory *m_factory;

    ChannelSList      m_channels;
    FilterSList       m_filters;
    SampleAdapterSList       m_samples;
    SegmentationSList m_segmentations;
    ClassificationSPtr      m_classification;

    QList<QDir>          m_tmpDirs;
    RelationshipGraphPtr m_relations;

    unsigned int m_lastId;
    bool         m_changed;
    bool         m_isTraceable;
  };

  typedef ModelAdapter *               EspinaModelPtr;
  typedef boost::shared_ptr<ModelAdapter> EspinaModelSPtr;

} // namespace EspINA

#endif // ESPINA_MODEL_ADAPTER_H