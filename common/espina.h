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

class IRenderable;
class ProcessingTrace;
class TaxonomyNode;
class Product;

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
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    // Sample managing
    Product *activeSample() {return m_samples.first();}

    // Segmentation managing
    QList<Product *> segmentations(const TaxonomyNode* taxonomy, bool recursive = false) const;
    
    // Taxonomy managin
    TaxonomyNode *taxonomy() {return m_tax;}

public slots:
    //! Add a new sample (used by the UI)
    void addSample(Product *sample);

    //! Add a new segmentation (used by the plugins)
    void addSegmentation(Product *seg);
    
    //! Set which is the taxonomy defined by the user
    void setUserDefindedTaxonomy(const QModelIndex &index);

signals:
    void render(IRenderable *product);
    void sliceRender(IRenderable *product);

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
    QModelIndex index(TaxonomyNode *node) const;
    
    //! Return the number of segmentations which belong to tax
    int numOfSegmentations(TaxonomyNode *tax) const;
    //! Return the number of subtaxonomies which belong to tax
    int numOfSubTaxonomies(TaxonomyNode *tax) const;

private:
    TaxonomyNode *m_tax;
    QString m_newSegType; // The type for new segmentations
    QMap<QString, QList<Product *> > m_segmentations;
    QList<Product *> m_samples;
    ProcessingTrace *m_analysis;

    static EspINA *m_singleton;
};

#endif // ESPINA_H
