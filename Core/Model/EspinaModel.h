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


//----------------------------------------------------------------------------
// File:    EspinaModel.h
// Purpose: Provide a model to centralize all data required by the application
//          Notify different views about changes in data
//----------------------------------------------------------------------------
#ifndef ESPinaModelMODEL_H
#define ESPinaModelMODEL_H

#include <QAbstractItemModel>
#include <QDir>

#include "Core/Model/RelationshipGraph.h"
#include "Filter.h"
#include "Channel.h"
#include "Segmentation.h"
#include <Core/IO/EspinaIO.h>

namespace EspINA
{
  /// Current Model arranges elements in the following way:
  /// QModelIndex() (invalid index/model root index)
  /// - TaxonomyRoot
  ///   - TaxonomyElement1
  ///     - Sub-TaxonomyElement1-1
  ///     - ...
  ///   - TaxonomyElement2
  ///     - ...
  ///   - ...
  /// - SampleRoot
  ///   - Sample1
  ///   - ...
  /// - ChannelRoot
  ///   - Channel1
  ///   - ...
  /// - SegmentationRoot
  ///   - Segmentation1
  ///   - ...
  /// - FilterRoot
  ///   - Filter1
  ///   - ...

  class EspinaFactory;

  class EspinaModel
  : public QAbstractItemModel
  {
    Q_OBJECT
  public:
    explicit EspinaModel(EspinaFactory *factory);
    virtual ~EspinaModel();

    EspinaFactory *factory() const
    { return m_factory; }

    void reset();

    // Implement QAbstractItemModel Interface
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual QMap< int, QVariant > itemData(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex index(ModelItemPtr item) const;
    QModelIndex index(ModelItemSPtr item) const;

    // Special Nodes of the model to refer different roots
    QModelIndex taxonomyRoot() const;
    QModelIndex taxonomyIndex(TaxonomyElementPtr node) const;
    QModelIndex taxonomyIndex(TaxonomyElementSPtr node) const;

    QModelIndex sampleRoot() const;
    QModelIndex sampleIndex(SamplePtr       sample) const;
    QModelIndex sampleIndex(SampleSPtr sample) const;

    QModelIndex channelRoot() const;
    QModelIndex channelIndex(ChannelPtr       channel) const;
    QModelIndex channelIndex(ChannelSPtr channel) const;

    QModelIndex segmentationRoot() const;
    QModelIndex segmentationIndex(SegmentationPtr       seg) const;
    QModelIndex segmentationIndex(SegmentationSPtr seg) const;

    QModelIndex filterRoot() const;
    QModelIndex filterIndex(FilterPtr       filter) const;
    QModelIndex filterIndex(FilterSPtr filter) const;

    bool hasChanged() const {return m_changed;}

    void markAsChanged() {m_changed = true;}

    void markAsSaved() {m_changed = false;}

    // Taxonomies
    /// Returns the taxonomy used by the analyzer
    void setTaxonomy(TaxonomySPtr tax);
    const TaxonomySPtr taxonomy() const {return m_tax;}
    void addTaxonomy(TaxonomySPtr tax);
    TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementPtr  parent, const QString &name);
    TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name);
    void addTaxonomyElement   (TaxonomyElementSPtr parent, TaxonomyElementSPtr element);
    void removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element);

    // Samples
    void addSample(SampleSPtr    sample  );
    void addSample(SampleSPtrList   samples );
    void removeSample(SampleSPtr sample  );
    SampleSPtrList samples() const
    { return m_samples; }

    // Channels
    void addChannel   (ChannelSPtr   channel  );
    void addChannel   (ChannelSList  channels );
    void removeChannel(ChannelSPtr   channel  );
    ChannelSList channels() const
    { return m_channels; }

    // Segmentations
    void addSegmentation   (SegmentationSPtr   segmentation  );
    void addSegmentation   (SegmentationSList  segmentations );
    void removeSegmentation(SegmentationSPtr   segmentation  );
    void removeSegmentation(SegmentationSList  segmentations );
    SegmentationSList segmentations() const
    { return m_segmentations; }

    // Filters
    void addFilter   (FilterSPtr  filter  );
    void addFilter   (FilterSPtrList filters );
    void removeFilter(FilterSPtr  filter  );
    FilterSPtrList filters() const
    { return m_filters; }

    void changeTaxonomy(SegmentationSPtr    segmentation,
                        TaxonomyElementSPtr taxonomy);

    void changeTaxonomyParent(TaxonomyElementSPtr subTaxonomy,
                              TaxonomyElementSPtr parent);


    //---------------------------------------------------------------------------
    /************************* Relationships API *******************************/
    //---------------------------------------------------------------------------
    void addRelation   (ModelItemSPtr   ancestor,
                        ModelItemSPtr   succesor,
                        const QString &relation);
    void removeRelation(ModelItemSPtr   ancestor,
                        ModelItemSPtr   succesor,
                        const QString &relation);

    ModelItemSList relatedItems(ModelItemPtr   item,
                                     RelationType   relType,
                                     const QString &relName = "");
    RelationList relations(ModelItemPtr   item,
                           const QString &relName = "");

    RelationshipGraphPtr relationships()
    { return m_relations; }

    // TODO 2012-12-17 Move to EspinaIO
    void serializeRelations(std::ostream& stream,
                            RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);
    bool loadSerialization (std::istream &stream,
                            QDir tmpDir,
                            EspinaIO::ErrorHandler *handler = NULL,
                            RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------
    ModelItemSPtr find(ModelItemPtr item);

    TaxonomyElementSPtr findTaxonomyElement(ModelItemPtr       item           );
    TaxonomyElementSPtr findTaxonomyElement(TaxonomyElementPtr taxonomyElement);

    SampleSPtr findSample(ModelItemPtr item  );
    SampleSPtr findSample(SamplePtr    sample);

    ChannelSPtr findChannel(ModelItemPtr item   );
    ChannelSPtr findChannel(ChannelPtr   channel);

    SegmentationSPtr findSegmentation(ModelItemPtr    item        );
    SegmentationSPtr findSegmentation(SegmentationPtr segmentation);

    FilterSPtr findFilter(ModelItemPtr item  );
    FilterSPtr findFilter(FilterPtr    filter);

    // signal emission methods, used by undo commands to signal finished operations.
    void emitSegmentationAdded(SegmentationSList);
    void emitChannelAdded(ChannelSList);

  signals:
    void taxonomyAdded  (TaxonomySPtr taxonomy);
    void taxonomyRemoved(TaxonomySPtr taxonomy);

    void sampleAdded  (SampleSPtr samples);
    void sampleRemoved(SampleSPtr samples);

    void channelAdded  (ChannelSPtr channel);
    void channelRemoved(ChannelSPtr channel);

    void segmentationAdded  (SegmentationSPtr segmentations);
    void segmentationRemoved(SegmentationSPtr segmentations);

    void filterAdded  (FilterSPtr filter);
    void filterRemoved(FilterSPtr filter);

  private slots:
    void itemModified(ModelItemPtr item);

  private:
    void addTaxonomy(TaxonomyElementSPtr root);

    void addSampleImplementation   (SampleSPtr sample);
    void removeSampleImplementation(SampleSPtr sample);

    void addChannelImplementation   (ChannelSPtr channel);
    void removeChannelImplementation(ChannelSPtr channel);

    void addSegmentationImplementation   (SegmentationSPtr segmentation);
    void removeSegmentationImplementation(SegmentationSPtr segmentation);

    void addFilterImplementation   (FilterSPtr filter);
    void removeFilterImplementation(FilterSPtr filter);

  private:
    EspinaFactory *m_factory;

    ChannelSList      m_channels;
    FilterSPtrList    m_filters;
    SampleSPtrList    m_samples;
    SegmentationSList m_segmentations;
    TaxonomySPtr      m_tax;

    QList<QDir>          m_tmpDirs;
    RelationshipGraphPtr m_relations;

    unsigned int m_lastId;
    bool         m_changed;
  };

  typedef EspinaModel *               EspinaModelPtr;
  typedef QSharedPointer<EspinaModel> EspinaModelSPtr;

} // namespace EspINA

#endif // ESPinaModelMODEL_H
