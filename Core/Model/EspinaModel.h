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
  class IEspinaModel
  {
  public:
    virtual ~IEspinaModel() {}

    virtual EspinaFactory *factory() const = 0;

    virtual bool isTraceable() const = 0;
    virtual void setTraceable(bool traceable) = 0;

    //---------------------------------------------------------------------------
    /************************* Model Item API *******************************/
    //---------------------------------------------------------------------------
    // Returns the taxonomy used by the analyzer
    virtual void setTaxonomy(TaxonomySPtr tax) = 0;
    virtual const TaxonomySPtr taxonomy() const = 0;
    virtual void addTaxonomy(TaxonomySPtr tax) = 0;
    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementPtr  parent, const QString &name) = 0;
    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name) = 0;
    virtual void addTaxonomyElement   (TaxonomyElementSPtr parent, TaxonomyElementSPtr element) = 0;
    virtual void removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element) = 0;

    // Samples
    virtual void addSample(SampleSPtr    sample ) = 0;
    virtual void addSample(SampleSList   samples) = 0;
    virtual void removeSample(SampleSPtr sample ) = 0;
    virtual SampleSList samples() const = 0;

    // Channels
    virtual void addChannel   (ChannelSPtr   channel  ) = 0;
    virtual void addChannel   (ChannelSList  channels ) = 0;
    virtual void removeChannel(ChannelSPtr   channel  ) = 0;
    virtual ChannelSList channels() const = 0;

    // Segmentations
    virtual void addSegmentation   (SegmentationSPtr   segmentation  ) = 0;
    virtual void addSegmentation   (SegmentationSList  segmentations ) = 0;
    virtual void removeSegmentation(SegmentationSPtr   segmentation  ) = 0;
    virtual void removeSegmentation(SegmentationSList  segmentations ) = 0;
    virtual SegmentationSList segmentations() const = 0;

    // Filters
    virtual void addFilter   (FilterSPtr     filter  ) = 0;
    virtual void addFilter   (FilterSList filters ) = 0;
    virtual void removeFilter(FilterSPtr     filter  ) = 0;
    virtual FilterSList filters() const = 0;

    //---------------------------------------------------------------------------
    /************************* Relationships API *******************************/
    //---------------------------------------------------------------------------
    virtual void addRelation   (ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation) = 0;

    virtual void removeRelation(ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation) = 0;

    virtual ModelItemSList relatedItems(ModelItemPtr   item,
                                        RelationType   relType,
                                        const QString &relName = "") = 0;

    virtual RelationList relations(ModelItemPtr   item,
                                   const QString &relName = "") = 0;

    virtual RelationshipGraphPtr relationships() = 0;

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------
    virtual ModelItemSPtr find(ModelItemPtr item) = 0;

    virtual TaxonomyElementSPtr findTaxonomyElement(ModelItemPtr       item           ) = 0;
    virtual TaxonomyElementSPtr findTaxonomyElement(TaxonomyElementPtr taxonomyElement) = 0;

    virtual SampleSPtr findSample(ModelItemPtr item  ) = 0;
    virtual SampleSPtr findSample(SamplePtr    sample) = 0;

    virtual ChannelSPtr findChannel(ModelItemPtr item   ) = 0;
    virtual ChannelSPtr findChannel(ChannelPtr   channel) = 0;

    virtual SegmentationSPtr findSegmentation(ModelItemPtr    item        ) = 0;
    virtual SegmentationSPtr findSegmentation(SegmentationPtr segmentation) = 0;

    virtual FilterSPtr findFilter(ModelItemPtr item  ) = 0;
    virtual FilterSPtr findFilter(FilterPtr    filter) = 0;
  };

  /// Model elements are arranged in the following way:
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
  , public IEspinaModel
  {
    Q_OBJECT
  public:
    explicit EspinaModel(EspinaFactory *factory);
    virtual ~EspinaModel();

    virtual EspinaFactory *factory() const
    { return m_factory; }

    virtual bool isTraceable() const;
    virtual void setTraceable(bool traceable) { m_isTraceable = traceable; }

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
    virtual void setTaxonomy(TaxonomySPtr tax);
    virtual const TaxonomySPtr taxonomy() const {return m_tax;}
    virtual void addTaxonomy(TaxonomySPtr tax);
    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementPtr  parent, const QString &name);
    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name);
    virtual void addTaxonomyElement   (TaxonomyElementSPtr parent, TaxonomyElementSPtr element);
    virtual void removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element);

    // Samples
    virtual void addSample(SampleSPtr    sample  );
    virtual void addSample(SampleSList   samples );
    virtual void removeSample(SampleSPtr sample  );
    virtual SampleSList samples() const
    { return m_samples; }

    // Channels
    virtual void addChannel   (ChannelSPtr   channel  );
    virtual void addChannel   (ChannelSList  channels );
    virtual void removeChannel(ChannelSPtr   channel  );
    virtual ChannelSList channels() const
    { return m_channels; }

    // Segmentations
    virtual void addSegmentation   (SegmentationSPtr   segmentation  );
    virtual void addSegmentation   (SegmentationSList  segmentations );
    virtual void removeSegmentation(SegmentationSPtr   segmentation  );
    virtual void removeSegmentation(SegmentationSList  segmentations );
    virtual SegmentationSList segmentations() const
    { return m_segmentations; }

    // Filters
    virtual void addFilter   (FilterSPtr  filter  );
    virtual void addFilter   (FilterSList filters );
    virtual void removeFilter(FilterSPtr  filter  );
    virtual FilterSList filters() const
    { return m_filters; }

    void changeTaxonomy(SegmentationSPtr    segmentation,
                        TaxonomyElementSPtr taxonomy);

    void changeTaxonomyParent(TaxonomyElementSPtr subTaxonomy,
                              TaxonomyElementSPtr parent);


    //---------------------------------------------------------------------------
    /************************* Relationships API *******************************/
    //---------------------------------------------------------------------------
    virtual void addRelation   (ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation);
    virtual void removeRelation(ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation);

    virtual ModelItemSList relatedItems(ModelItemPtr   item,
                                        RelationType   relType,
                                     const QString &relName = "");
    virtual RelationList relations(ModelItemPtr   item,
                                   const QString &relName = "");

    virtual RelationshipGraphPtr relationships()
    { return m_relations; }

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------
    virtual ModelItemSPtr find(ModelItemPtr item);

    virtual TaxonomyElementSPtr findTaxonomyElement(ModelItemPtr       item           );
    virtual TaxonomyElementSPtr findTaxonomyElement(TaxonomyElementPtr taxonomyElement);

    virtual SampleSPtr findSample(ModelItemPtr item  );
    virtual SampleSPtr findSample(SamplePtr    sample);

    virtual ChannelSPtr findChannel(ModelItemPtr item   );
    virtual ChannelSPtr findChannel(ChannelPtr   channel);

    virtual SegmentationSPtr findSegmentation(ModelItemPtr    item        );
    virtual SegmentationSPtr findSegmentation(SegmentationPtr segmentation);

    virtual FilterSPtr findFilter(ModelItemPtr item  );
    virtual FilterSPtr findFilter(FilterPtr    filter);

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
    FilterSList       m_filters;
    SampleSList       m_samples;
    SegmentationSList m_segmentations;
    TaxonomySPtr      m_tax;

    QList<QDir>          m_tmpDirs;
    RelationshipGraphPtr m_relations;

    unsigned int m_lastId;
    bool         m_changed;
    bool         m_isTraceable;
  };

  typedef EspinaModel *               EspinaModelPtr;
  typedef QSharedPointer<EspinaModel> EspinaModelSPtr;

} // namespace EspINA

#endif // ESPinaModelMODEL_H
