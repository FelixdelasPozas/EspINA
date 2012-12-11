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

class Channel;
class EspinaFactory;
class Filter;
class ModelItem;
class Sample;
class Segmentation;
class Taxonomy;
class TaxonomyElement;

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
  explicit EspinaModel(EspinaFactory *factory, QObject* parent = 0);
  virtual ~EspinaModel();

  EspinaFactory *factory() const
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
  QModelIndex index(ModelItem *item) const;

  // Special Nodes of the model to refer different roots
  QModelIndex taxonomyRoot() const;
  QModelIndex taxonomyIndex(TaxonomyElement *node) const;

  QModelIndex sampleRoot() const;
  QModelIndex sampleIndex(Sample *sample) const;

  QModelIndex channelRoot() const;
  QModelIndex channelIndex(Channel *channel) const;

  QModelIndex segmentationRoot() const;
  QModelIndex segmentationIndex(Segmentation *seg) const;

  QModelIndex filterRoot() const;
  QModelIndex filterIndex(Filter *filter) const;

  bool hasChanged() const {return m_changed;}
  void markAsChanged() {m_changed = true;}
  void markAsSaved(){m_changed = false;}


  // Taxonomies
  /// Returns the taxonomy used by the analyzer
  void setTaxonomy(Taxonomy *tax);
  Taxonomy * const taxonomy() const {return m_tax;}
  void addTaxonomy(Taxonomy *tax);
  QModelIndex addTaxonomyElement(const QModelIndex &parent, QString qualifiedName);
  void addTaxonomyElement(QString qualifiedName);
  void removeTaxonomyElement(const QModelIndex &index);
  void removeTaxonomyElement(QString qualifiedName);

  // Samples
  void addSample(Sample *sample);
  void addSample(QList<Sample *> samples);
  /// Remove @sample and all channels and segmentations associated with it
  void removeSample(Sample *sample);

  // Channels
  void addChannel(Channel *channel);
  void removeChannel(Channel *channel);
  QList<Channel *> channels() const {return m_channels;}

  // Segmentations
  void addSegmentation(Segmentation *seg);
  void addSegmentation(QList<Segmentation *> segs);
  void removeSegmentation(Segmentation *seg);
  void removeSegmentation(QList<Segmentation *> segs);
  void changeTaxonomy(Segmentation *seg, TaxonomyElement *taxonomy);
  QList<Segmentation *> segmentations() const {return m_segmentations;}

  void addFilter(Filter *filter);
  void removeFilter(Filter *filter);
  QList<Filter *> filters() const {return m_filters;}

  void addRelation(ModelItem *ancestor,
                   ModelItem *succesor,
                   QString relation);
  void removeRelation(ModelItem *ancestor,
                      ModelItem *succesor,
                      QString relation);

  RelationshipGraph *relationships() {return m_relations;}

  void serializeRelations(std::ostream& stream, RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);
  bool loadSerialization (std::istream &stream, QDir tmpDir, RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

private slots:
  void itemModified(ModelItem *item);

private:
  void addTaxonomy(TaxonomyElement *tax);

private:
  EspinaFactory        *m_factory;
  Taxonomy             *m_tax;
  QList<Sample *>       m_samples;
  QList<Channel *>      m_channels;
  QList<Segmentation *> m_segmentations;
  QList<Filter *>       m_filters;

  QList<QDir>           m_tmpDirs;
  RelationshipGraph    *m_relations;

  unsigned int          m_lastId;
  bool                  m_changed;
};

#endif // ESPinaModelMODEL_H
