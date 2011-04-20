/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef ESPINA_H
#define ESPINA_H

#include <QModelIndex>

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <pqProxy.h>
#include "products.h"
#include "pqPipelineSource.h"

class IRenderable;
class ProcessingTrace;
class TaxonomyNode;
class Product;
class Sample;
class Segmentation;

//! Espina Interactive Neuron Analyzer
//! The logic model for the application
class EspINA : public QAbstractItemModel
{
    Q_OBJECT
public:
    // Defines Factory Type: It should be use to configure the behaviour of some parts
    // of the program
    typedef enum {ELECTRONE, OPTICAL} Microscopy;
public:
    //! Singleton Interface
    static EspINA *instance();
    virtual ~EspINA();

    //! Implement QAbstractItemModel Interface
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    
    // Special Nodes of the model to refer different roots
    QModelIndex taxonomyRoot() const;
    QModelIndex sampleRoot() const;
    QModelIndex segmentationRoot() const;
    
    //! Returns the active sample: Only one sample can be active at a time. This is the one
    //! that is shown in the Slice Views (Plane Views)
    Sample *activeSample() {return m_activeSample;}
    //! Returs the QModelIndex of a given @sample
    QModelIndex sampleIndex(Sample *sample) const;

    // Segmentation managing
    //! Returns the list of segmentations belonging to @taxonomy. If @recursive returns also
    //! the segmentations belonging to its subtaxonomies
    QList<Segmentation *> segmentations(const TaxonomyNode* taxonomy, bool recursive = false) const;
    //! Returns the list of segmentations belonging to @sample.
    QList<Segmentation *> segmentations(const Sample* sample) const;
    
    // Taxonomy managing
    //! Returns the taxonomy used by the analyzer
    TaxonomyNode *taxonomy() {return m_tax;}
    //! Returns the QModelIndex of a given @node
    QModelIndex taxonomyIndex(TaxonomyNode *node) const;
    
    //! Returns the QModelIndex of a given @seg
    QModelIndex segmentationIndex(Segmentation *seg) const;
    
    //! Openning .trace In the future .seg (.trace + .mha) (used by the UI)
    void loadFile(QString& filePath, pqServer* server = NULL);
    void saveFile(QString& filePath, pqServer* server = NULL);
    
public slots:
    //TODO: Check if private? Now it's only used by Espina
    void addSample(EspinaProxy* source, int portNumber, QString& filePath);

    //! Add a new segmentation (used by the plugins)
    void addSegmentation(Segmentation *seg);
    //! Remove a segmentation (used by the UI)
    void removeSegmentation(Segmentation *seg);
    
    //! Set which is the taxonomy that will be used for new segmentations
    //! when plugins can't guess their type
    void setUserDefindedTaxonomy(const QString &taxName);

    //! Debug slot for plugins manage
    void onProxyCreated(pqProxy* p);
    
signals:
    //! 
    void focusSampleChanged(Sample *);
  
protected:
    explicit EspINA(QObject* parent = 0);

private:
    //! 
    void loadTaxonomy();//TODO: Replace with factory
    //TaxonomyNode *indexNode(const QModelIndex &index) const;
    //! Returns wheter or not node is a taxonomy leaf node
    bool isLeaf(TaxonomyNode *node) const;
    //! Return the QModelIndex for a Taxonomy node given by
    //! the row and column of the node whithin its parent index
    //! and a pointer to the node itself
    
    //! Return the number of segmentations which belong to tax
    int numOfSegmentations(TaxonomyNode *tax) const;
    //! Return the number of subtaxonomies which belong to tax
    int numOfSubTaxonomies(TaxonomyNode *tax) const;

private:
    TaxonomyNode *m_newSegType; // The type for new segmentations
    Sample *m_activeSample;
    TaxonomyNode *m_tax;
    QList<Sample *> m_samples;
    QList<Segmentation *> m_segmentations;
    ProcessingTrace *m_analysis;

    QMap<const TaxonomyNode *, QList<Segmentation *> > m_taxonomySegs;
    QMap<const Sample *, QList<Segmentation *> > m_sampleSegs;

    static EspINA *m_singleton;
};

#endif // ESPINA_H
