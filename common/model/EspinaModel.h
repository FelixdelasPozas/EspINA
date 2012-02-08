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

class Channel;
class Sample;
class Segmentation;
class Taxonomy;
class TaxonomyNode;
// #include <QObject>
// #include <QMap>
// #include <QList>
// #include <QString>
// #include <pqProxy.h>
// #include "products.h"
// #include "pqPipelineSource.h"
// #include <QDir>

// class IRenderable;
// class ProcessingTrace;
// class TaxonomyNode;
// class Product;
// class Sample;
// class Segmentation;
// 
/// Espina Interactive Neuron Analyzer
/// The logic model for the application
class EspinaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
  explicit EspinaModel(QObject* parent = 0);
  virtual ~EspinaModel();

  void clear();

  // Implement QAbstractItemModel Interface
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  //     virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  //     virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  // Special Nodes of the model to refer different roots
  QModelIndex taxonomyRoot() const;
  QModelIndex taxonomyIndex(TaxonomyNode* node) const;

  QModelIndex sampleRoot() const;
  QModelIndex sampleIndex(Sample *sample) const;

  QModelIndex channelRoot() const;
  QModelIndex channelIndex(Channel *channel) const;

  QModelIndex segmentationRoot() const;
  QModelIndex segmentationIndex(Segmentation *seg) const;

  // Taxonomies
  /// Returns the taxonomy used by the analyzer
  void setTaxonomy(Taxonomy *tax);
  Taxonomy *const taxonomy() const {return m_tax;}
  QModelIndex addTaxonomyElement(const QModelIndex &parent, QString qualifiedName);
  void addTaxonomyElement(QString qualifiedName);
  void removeTaxonomyElement(const QModelIndex &index);
  void removeTaxonomyElement(QString qualifiedName);
//     TaxonomyNode *taxonomyParent(TaxonomyNode *node);

  // Samples
  void addSample(Sample *sample);
  /// Remove @sample and all channels and segmentations associated with it
  void removeSample(Sample *sample);

  // Channels
  void addChannel(Sample *sample, Channel *channel);
  void removeChannel(Sample *sample, Channel *channel);

  // Segmentations
  /// Add a new segmentation (used by the plugins)
  void addSegmentation(Segmentation *seg);
  /// Remove a segmentation (used by the UI)
  void removeSegmentation(Segmentation *seg);

//     //! Returns the list of segmentations belonging to @taxonomy. If @recursive returns also
//     //! the segmentations belonging to its subtaxonomies
//     QList<Segmentation *> segmentations(const TaxonomyNode* taxonomy, bool recursive = false) const;
//     //! Returns the list of segmentations belonging to @sample.
//     QList<Segmentation *> segmentations(const Sample* sample) const;
//
//     void changeTaxonomy(Segmentation *seg, TaxonomyNode *newTaxonomy);
//     void changeTaxonomy(Segmentation* seg, const QString& taxName);

//
//     int requestId(int suggestedId);
//
//     void changeId(Segmentation *seg, int id);
//     void setLastUsedId(int id) {m_nextValidSegId = id+1;}
//
//     Segmentation* segmentation(QString& segId);
//
//     //void assignTaxonomy(QString& taxName, QString& segId);
//
//     //! Openning .trace In the future .seg (.trace + .mha) (used by the UI)
//         void saveFile(QString& filePath, pqServer* server = NULL);
//
// public slots:
//     void loadFile(QString filePath, QString method);
//     //TODO: Check if private? Now it's only used by Espina
//     void addSample(Sample *sample);
//
//
//     void removeSamples();
//
//
//     //! Set which is the taxonomy that will be used for new segmentations
//     //! when plugins can't guess their type
//     void setUserDefindedTaxonomy(const QString &taxName);
//
//     //! Debug slot to manage plugins
//     void onProxyCreated(pqProxy* p);
//     void destroyingProxy(pqProxy* p);
//
//     //! Manage the pqPipelineSources loaded with pqLoadReaction
//     //TODO: Refactor
//     void loadSource(pqPipelineSource* proxy);
//
//     //! Clear all the Espina Model. It removes the Samples, Segmentations
//     //! and the Taxonomy.
//     void clear();
//
// signals:
//     //!
//     void focusSampleChanged(Sample *);
//
//     void resetTaxonomy();
//
// protected slots:
//   void internalSegmentationUpdate(Segmentation *seg);

private:
  Taxonomy             *m_tax;
  QList<Sample *>       m_samples; //TODO: use DB instead
  QList<Channel *>      m_channels; //TODO: use DB instead
  QList<Segmentation *> m_segmentations; //TODO: use DB instead

//     //!
//     void removeTaxonomy();
//     void loadTaxonomy();//TODO: Replace with factor
//     //TaxonomyNode *indexNode(const QModelIndex &index) const;
//     //! Returns wheter or not node is a taxonomy leaf node
//     bool isLeaf(TaxonomyNode *node) const;
//     //! Return the QModelIndex for a Taxonomy node given by
//     //! the row and column of the node whithin its parent index
//     //! and a pointer to the node itself
//
//     //! Return the number of segmentations which belong to tax
//     int numOfSegmentations(TaxonomyNode *tax) const;
//     //! Return the number of subtaxonomies which belong to tax
//     int numOfSubTaxonomies(TaxonomyNode* tax) const;
//
//     //! Save a segmentation in the active server in a file which name
//     //! corresponds to the id of the segmentation
//     bool saveSegmentation( Segmentation* seg, QDir prefixFilePath);
//
//     int nextSegmentationId() {return m_nextValidSegId++;}
//
// private:
//     TaxonomyNode *m_newSegType; // The type for new segmentations
//     Sample *m_activeSample;
//
//     QList<Sample *> m_samples;
//     QList<Segmentation *> m_segmentations;
//     //! It contains all the pipeline of filters, segmentations and samples
//     ProcessingTrace *m_analysis;
//
//     QMap<const TaxonomyNode *, QList<Segmentation *> > m_taxonomySegs;
//
//
//     int m_nextValidSegId;
};

#endif // ESPinaModelMODEL_H
