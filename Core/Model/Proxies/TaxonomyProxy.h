/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TAXONOMYPROXY_H
#define TAXONOMYPROXY_H

#include <QAbstractProxyModel>

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  /// Group by Taxonomy Espina Proxy
  class TaxonomyProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    TaxonomyProxy(QObject *parent=0);
    virtual ~TaxonomyProxy();

    virtual void setSourceModel(EspinaModel *sourceModel);

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {return 1;}
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

    /// Drag & Drop support
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDropActions() const {return Qt::MoveAction;}
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    int numSegmentations(QModelIndex taxIndex, bool recursive = false) const;
    int numTaxonomies(QModelIndex taxIndex) const;
    QModelIndexList segmentations(QModelIndex taxIndex, bool recursive=false) const;

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);

  signals:
    void taxonomiesDragged(TaxonomyElementList sources, TaxonomyElementPtr parent);
    void segmentationsDragged(SegmentationList sources, TaxonomyElementPtr taxonomy);

  protected:
    bool indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result);
    SegmentationPtr findSegmentation(QString tooltip);
    QModelIndexList sourceIndices(const QModelIndex& parent, int start, int end) const;
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;

    void addTaxonomicalElement(TaxonomyElementPtr taxonomy);
    void removeTaxonomy(TaxonomyElementPtr taxonomy);
    int numSegmentations(TaxonomyElementPtr taxonomy, bool recursive = false) const;
    int numTaxonomies(TaxonomyElementPtr taxonomy) const;

  private:
    EspinaModel *m_model;
    // Keep a reference to the taxonomies which belong to the root taxonomy
    TaxonomyElementList m_rootTaxonomies;
    // We need to rely on our own row count for each item in the proxy's model
    // If we rely on the source's model, there are some inconsistencies during
    // rows insertion/deletion
    mutable QMap<TaxonomyElementPtr, int          > m_numTaxonomies;
    mutable QMap<TaxonomyElementPtr, ModelItemList> m_numSegmentations;
  };

} // namespace EspINA

#endif // TAXONOMYPROXY_H