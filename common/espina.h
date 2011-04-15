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
#include "traceNodes.h"
#include "pqPipelineSource.h"

class IRenderable;
class ProcessingTrace;
class TaxonomyNode;
class Product;
class Sample;
class Segmentation;

//! Espina Interactive Neuron Analyzer
class EspINA : public QAbstractItemModel
{
    Q_OBJECT
public:
    // Defines Factory Type
    typedef enum {ELECTRONE, OPTICAL} Microscopy;
public:
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
    
    // Special Nodes
    QModelIndex taxonomyRoot() const;
    QModelIndex sampleRoot() const;
    QModelIndex segmentationRoot() const;
    
    // Sample managing
    Sample *activeSample() {return m_activeSample;}
    QModelIndex sampleIndex(Sample *sample) const;

    // Segmentation managing
    QList<Segmentation *> segmentations(const TaxonomyNode* taxonomy, bool recursive = false) const;
    QList<Segmentation *> segmentations(const Sample* sample) const;
    
    // Taxonomy managin
    TaxonomyNode *taxonomy() {return m_tax;}
    QModelIndex taxonomyIndex(TaxonomyNode *node) const;
    
    QModelIndex segmentationIndex(Segmentation *seg) const;
    
    //! Openning .mha, .trace or .seg (.trace + .mha) file (used by the UI)
    //! After a paraviews open.
    void loadFile(EspinaProxy* proxy);
    void saveTrace(QString filePath);
    
public slots:
    //! Add a new sample (used by the UI -> not anymore)
    void addSample(EspinaProxy* source, int portNumber, QString& filePath);

    //! Add a new segmentation (used by the plugins)
    void addSegmentation(Segmentation *seg);
    void removeSegmentation(Segmentation *seg);
    
    //! Set which is the taxonomy defined by the user
    void setUserDefindedTaxonomy(const QString &taxName);

    //! Debug slot for plugins manage
    void onProxyCreated(pqProxy* p);
    
signals:
    void focusSampleChanged(Sample *);
  
protected:
    explicit EspINA(QObject* parent = 0);

private:
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
