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
  class EspinaModel
  : public QAbstractItemModel
  {
    Q_OBJECT
  public:
    explicit EspinaModel(EspinaFactoryPtr factory, QObject* parent = 0);
    virtual ~EspinaModel();

    EspinaFactoryPtr factory() const
    { return m_factory; }

    void reset();

    // Implement QAbstractItemModel Interface
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex index(ModelItemPtr item) const;

    // Special Nodes of the model to refer different roots
    QModelIndex taxonomyRoot() const;
    QModelIndex taxonomyIndex(TaxonomyElementPtr node) const;

    QModelIndex sampleRoot() const;
    QModelIndex sampleIndex(SamplePtr sample) const;

    QModelIndex channelRoot() const;
    QModelIndex channelIndex(ChannelPtr channel) const;

    QModelIndex segmentationRoot() const;
    QModelIndex segmentationIndex(SegmentationPtr seg) const;

    QModelIndex filterRoot() const;
    QModelIndex filterIndex(FilterPtr filter) const;

    bool hasChanged() const {return m_changed;}
    void markAsChanged() {m_changed = true;}
    void markAsSaved(){m_changed = false;}


    // Taxonomies
    /// Returns the taxonomy used by the analyzer
    void setTaxonomy(TaxonomyPtr tax);
    const TaxonomyPtr taxonomy() const {return m_tax;}
    void addTaxonomy(TaxonomyPtr tax);
    QModelIndex addTaxonomyElement(const QModelIndex &parent, QString qualifiedName);
    void addTaxonomyElement(QString qualifiedName);
    void removeTaxonomyElement(const QModelIndex &index);
    void removeTaxonomyElement(QString qualifiedName);

    // Samples
    void addSample(SamplePtr  sample);
    void addSample(SampleList samples);
    /// Remove @sample
    void removeSample(SamplePtr sample);
    const SampleList samples() const
    {return m_samples; }

    // Channels
    void addChannel(ChannelPtr    channel);
    void removeChannel(ChannelPtr channel);
    ChannelList channels() const
    {return m_channels;}

    // Segmentations
    void addSegmentation   (SegmentationPtr   seg);
    void addSegmentation   (SegmentationList segs);
    void removeSegmentation(SegmentationPtr   seg);
    void removeSegmentation(SegmentationList segs);
    SegmentationList segmentations() const
    {return m_segmentations;}

    // TODO: Undoable
    void changeTaxonomy(SegmentationPtr seg, TaxonomyElementPtr taxonomy);

    void addFilter(FilterPtr filter);
    void removeFilter(FilterPtr filter);
    FilterList filters() const
    {return m_filters;}

    void addRelation   (ModelItemPtr   ancestor,
                        ModelItemPtr   succesor,
                        const QString &relation);
    void removeRelation(ModelItemPtr   ancestor,
                        ModelItemPtr   succesor,
                        const QString &relation);

    RelationshipGraphPtr relationships()
    {return m_relations;}

    void serializeRelations(std::ostream& stream,
                            RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);
    bool loadSerialization (std::istream &stream,
                            QDir tmpDir,
                            RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

  private slots:
    void itemModified(ModelItemPtr item);

  private:
    void addTaxonomy(TaxonomyElementPtr tax);
    void addSegmentationImplementation(SegmentationPtr seg);

  private:
    EspinaFactoryPtr m_factory;

    ChannelList      m_channels;
    FilterList       m_filters;
    SampleList       m_samples;
    SegmentationList m_segmentations;
    TaxonomyPtr      m_tax;

    QList<QDir>          m_tmpDirs;
    RelationshipGraphPtr m_relations;

    unsigned int m_lastId;
    bool         m_changed;
  };

} // namespace EspINA

#endif // ESPinaModelMODEL_H
