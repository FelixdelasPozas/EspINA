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
// Purpose: Decide how to deal with different types of resources
//          Load/Save Files
//          Generate required models depending on the internal state
//----------------------------------------------------------------------------
#ifndef ESPinaModelMODEL_H
#define ESPinaModelMODEL_H

#include <QAbstractItemModel>


#include <QSharedPointer>

#include "common/model/Sample.h"
#include "common/model/Filter.h"
#include "common/model/Taxonomy.h"
#include "common/model/RelationshipGraph.h"

class Channel;
class Filter;
class ModelItem;
class RelationshipGraph;
class Segmentation;
class Taxonomy;
class TaxonomyNode;

/// Espina Interactive Neuron Analyzer
/// The logic model for the application
class EspinaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
  explicit EspinaModel(QObject* parent = 0);
  virtual ~EspinaModel();

  void reset();

  // Implement QAbstractItemModel Interface
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  //     virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  // Special Nodes of the model to refer different roots
  QModelIndex taxonomyRoot() const;
  QModelIndex taxonomyIndex(TaxonomyNode *node) const;

  QModelIndex sampleRoot() const;
  QModelIndex sampleIndex(Sample *sample) const;

  QModelIndex channelRoot() const;
  QModelIndex channelIndex(Channel *channel) const;

  QModelIndex segmentationRoot() const;
  QModelIndex segmentationIndex(Segmentation *seg) const;

  QModelIndex filterRoot() const;
  QModelIndex filterIndex(Filter *filter) const;


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
  Channel *channel(const QString id) const;

  // Segmentations
  void addSegmentation(Segmentation *seg);
  void addSegmentation(QList<Segmentation *> segs);
  void removeSegmentation(Segmentation *seg);
  QList<Segmentation *> segmentations() const {return m_segmentations;}

  void addFilter(Filter *filter);
  void removeFilter(Filter *filter);

  void addRelation(ModelItem *ancestor,
		   ModelItem *succesor,
		   QString relation);
  void removeRelation(ModelItem *ancestor,
		      ModelItem *succesor,
		      QString relation);

  RelationshipGraph *relationships() {return m_relations;}

  void serializeRelations(std::ostream& stream, RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);
  void loadSerialization (std::istream &stream, RelationshipGraph::PrintFormat format = RelationshipGraph::BOOST);

private:
  void addTaxonomy(TaxonomyNode *tax);
  QModelIndex index(ModelItem *item) const;

private:
  Taxonomy             *m_tax;
  QList<Sample *>       m_samples;
  QList<Channel *>      m_channels;
  QList<Segmentation *> m_segmentations;
  QList<Filter *>       m_filters;

  RelationshipGraph    *m_relations;

  unsigned int           m_lastId;
};

#endif // ESPinaModelMODEL_H
