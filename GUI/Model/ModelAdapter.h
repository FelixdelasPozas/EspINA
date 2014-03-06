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

#include <QAbstractItemModel>

#include "GUI/Model/SampleAdapter.h"
#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/ClassificationAdapter.h"
#include "GUI/Model/SegmentationAdapter.h"
#include "GUI/Model/FilterAdapter.h"
#include <GUI/ModelFactory.h>

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
  *   \brief Adapt analysis to Qt Model framework
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
    struct Existing_Item_Exception{};
    struct Existing_Relation_Exception{};
    struct Item_Not_Found_Exception {};
    struct Relation_Not_Found_Exception {};

  public:
    explicit ModelAdapter();
    virtual ~ModelAdapter();

    void setAnalysis(AnalysisSPtr analysis, ModelFactorySPtr factory);

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
    QModelIndex categoryIndex(CategoryAdapterPtr  category) const;
    QModelIndex categoryIndex(CategoryAdapterSPtr category) const;

    QModelIndex sampleRoot() const;
    QModelIndex sampleIndex(SampleAdapterPtr  sample) const;
    QModelIndex sampleIndex(SampleAdapterSPtr sample) const;

    QModelIndex channelRoot() const;
    QModelIndex channelIndex(ChannelAdapterPtr  channel) const;
    QModelIndex channelIndex(ChannelAdapterSPtr channel) const;

    QModelIndex segmentationRoot() const;
    QModelIndex segmentationIndex(SegmentationAdapterPtr  segmentation) const;
    QModelIndex segmentationIndex(SegmentationAdapterSPtr segmentation) const;

    // Classification
    void setClassification(ClassificationAdapterSPtr classification);

    const ClassificationAdapterSPtr classification() const;

    CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterPtr  parent=nullptr);
    CategoryAdapterSPtr createCategory(const QString& name, CategoryAdapterSPtr parent=CategoryAdapterSPtr());

    void addCategory   (CategoryAdapterSPtr category, CategoryAdapterSPtr parent);
    void removeCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

    //TODO 2013-10-21: Throw exception if they don't belong to the same classification
    void reparentCategory(CategoryAdapterSPtr category, CategoryAdapterSPtr parent);

    void add(SampleAdapterSPtr        sample);
    void add(SampleAdapterSList       samples);
    void add(ChannelAdapterSPtr       channel);
    void add(ChannelAdapterSList      channels);
    void add(SegmentationAdapterSPtr  segmentation);
    void add(SegmentationAdapterSList segmentations);

    void remove(SampleAdapterSPtr        sample);
    void remove(SampleAdapterSList       samples);
    void remove(ChannelAdapterSPtr       channel);
    void remove(ChannelAdapterSList      channels);
    void remove(SegmentationAdapterSPtr  segmentation);
    void remove(SegmentationAdapterSList segmentations);

    SampleAdapterSList samples() const
    { return m_samples; }

    ChannelAdapterSList channels() const
    { return m_channels; }

    SegmentationAdapterSList segmentations() const
    { return m_segmentations; }

    void setSegmentationCategory(SegmentationAdapterSPtr segmentation,
                                 CategoryAdapterSPtr     category);


    void addRelation(ItemAdapterSPtr     ancestor,
                     ItemAdapterSPtr     succesor,
                     const RelationName& relation);

    void addRelation(const Relation& relation); 

    void deleteRelation(ItemAdapterSPtr     ancestor,
                        ItemAdapterSPtr     succesor,
                        const RelationName& relation);

    void deleteRelation(const Relation& relation); 

    ItemAdapterSList relatedItems(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

    RelationList relations(ItemAdapterPtr item, RelationType type, const RelationName& filter = QString());

    // signal emission methods, used by undo commands to signal finished operations.
    void emitSegmentationsAdded(SegmentationAdapterSPtr segmentation);
    void emitSegmentationsAdded(SegmentationAdapterSList segmentations);
    void emitChannelAdded(ChannelAdapterSList);

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------
    ItemAdapterSPtr find(PersistentSPtr item);

//     virtual CategoryAdapterSPtr findCategory(ModelItemPtr       item           );
    CategoryAdapterSPtr smartPointer(CategoryAdapterPtr category);
// 
//     virtual SampleAdapterSPtr findSampleAdapter(ModelItemPtr item  );
    virtual SampleAdapterSPtr smartPointer(SampleAdapterPtr    sample);
// 
//     virtual ChannelSPtr findChannel(ModelItemPtr item   );
    virtual ChannelAdapterSPtr smartPointer(ChannelAdapterPtr channel);
// 
//     virtual SegmentationAdapterSPtr findSegmentation(ModelItemPtr    item        );
    virtual SegmentationAdapterSPtr smartPointer(SegmentationAdapterPtr segmentation);
// 
//     virtual FilterSPtr findFilter(ModelItemPtr item  );
//     virtual FilterSPtr findFilter(FilterPtr    filter);


  signals:
    void classificationAdded  (ClassificationAdapterSPtr classification);
    void classificationRemoved(ClassificationAdapterSPtr classification);

    void sampleAdded  (SampleAdapterSPtr samples);
    void sampleRemoved(SampleAdapterSPtr samples);

    void channelAdded  (ChannelAdapterSPtr channel);
    void channelRemoved(ChannelAdapterSPtr channel);

    void segmentationsAdded  (SegmentationAdapterSList segmentations);
    void segmentationsRemoved(SegmentationAdapterSList segmentations);

  private slots:
    void itemModified(ItemAdapterSPtr item);

  private:

    void addImplementation(SampleAdapterSPtr       sample);
    void addImplementation(ChannelAdapterSPtr      channel);
    void addImplementation(SegmentationAdapterSPtr segmentation);

    void removeImplementation(SampleAdapterSPtr       sample);
    void removeImplementation(ChannelAdapterSPtr      channel);
    void removeImplementation(SegmentationAdapterSPtr segmentation);

  private:
    AnalysisSPtr              m_analysis;
    SampleAdapterSList        m_samples;
    ChannelAdapterSList       m_channels;
    SegmentationAdapterSList  m_segmentations;
    ClassificationAdapterSPtr m_classification;
  };

  using ModelAdapterPtr  = ModelAdapter *;
  using ModelAdapterSPtr = std::shared_ptr<ModelAdapter>;

  ItemAdapterPtr EspinaGUI_EXPORT itemAdapter(const QModelIndex &index);

  bool isClassification(ItemAdapterPtr item);
  bool isCategory(ItemAdapterPtr item);
  bool isSample(ItemAdapterPtr item);
  bool isChannel(ItemAdapterPtr item);
  bool isSegmentation(ItemAdapterPtr item);

} // namespace EspINA

#endif // ESPINA_MODEL_ADAPTER_H
