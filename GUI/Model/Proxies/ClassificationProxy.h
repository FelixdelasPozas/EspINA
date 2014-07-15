/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_CLASSIFICATION_PROXY_H
#define ESPINA_CLASSIFICATION_PROXY_H

#include <QAbstractProxyModel>
#include <GUI/Model/ModelAdapter.h>

namespace EspINA
{
  /** \brief Group Segmentations by Category
   *
   */
  class EspinaGUI_EXPORT ClassificationProxy
  : public QAbstractProxyModel
  {
    Q_OBJECT
  public:
    ClassificationProxy(ModelAdapterSPtr sourceModel, QObject *parent=0);
    virtual ~ClassificationProxy();

    virtual void setSourceModel(ModelAdapterSPtr sourceModel);

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

    int numSegmentations(QModelIndex  categoryIndexx, bool recursive = false) const;
    int numSegmentations(CategoryAdapterPtr category, bool recursive = false) const;

    int numSubCategories(QModelIndex  categoryIndexx) const;
    int numSubCategories(CategoryAdapterPtr category) const;

    QModelIndexList         segmentations(QModelIndex  categoryIndexx, bool recursive = false) const;
    SegmentationAdapterList segmentations(CategoryAdapterPtr category, bool recursive = false) const;

  protected slots:
    void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);
    void sourceModelReset();

  signals:
    void categoriesDropped(CategoryAdapterList sources, CategoryAdapterPtr parent);
    void segmentationsDropped(SegmentationAdapterList sources, CategoryAdapterPtr category);

  protected:
    bool indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result);
    SegmentationAdapterPtr findSegmentation(QString tooltip);
    QModelIndexList sourceIndices(const QModelIndex& parent, int start, int end) const;
    QModelIndexList proxyIndices(const QModelIndex& parent, int start, int end) const;

    void addCategory(CategoryAdapterPtr category);
    void removeCategory(CategoryAdapterPtr category);

    // Return the index of the category in the proxy model
    QModelIndex categoryIndex(CategoryAdapterPtr category) const;

  private:
    CategoryAdapterPtr createProxyCategory(CategoryAdapterPtr sourceCategory);

    QModelIndex createSourceCategoryIndex(int row, int column, CategoryAdapterPtr category) const;

    CategoryAdapterPtr  toProxyPtr  (CategoryAdapterPtr sourceCategory) const;
    CategoryAdapterSPtr toProxySPtr (CategoryAdapterPtr sourceCategory) const;
    CategoryAdapterPtr  toSourcePtr (CategoryAdapterPtr proxyCategory) const;
    CategoryAdapterSPtr toSourceSPtr(CategoryAdapterPtr proxyCategory) const;

  private:
    ModelAdapterSPtr m_model;
    // Keep a reference to the categories which belong to the classification root
    CategoryAdapterList m_rootCategories;

    // We need to rely on our own row count for each item in the proxy's model
    // If we rely on the source's model, there are some inconsistencies during
    // rows insertion/deletion
    mutable QMap<CategoryAdapterPtr, CategoryAdapterPtr> m_sourceCategory;
    mutable ClassificationAdapterSPtr                    m_classification;
    mutable QMap<CategoryAdapterPtr, int            >    m_numCategories;
    mutable QMap<CategoryAdapterPtr, ItemAdapterList>    m_categorySegmentations;
    mutable QMap<CategoryAdapterPtr, Qt::CheckState >    m_categoryVisibility;
  };

} // namespace EspINA

#endif // ESPINA_CLASSIFICATION_PROXY_H
