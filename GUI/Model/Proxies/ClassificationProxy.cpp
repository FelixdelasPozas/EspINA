/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ClassificationProxy.h"

#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/ViewState.h>

// Qt
#include <QMimeData>
#include <QPixmap>
#include <QPainter>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

typedef QSet<ItemAdapterPtr > SegmentationSet;

enum DragSourceEnum {
  NoSource           = 0x0,
  SegmentationSource = 0x1,
  CategorySource     = 0x2,
  InvalidSource      = 0x3
};
Q_DECLARE_FLAGS(DragSource, DragSourceEnum);

Q_DECLARE_OPERATORS_FOR_FLAGS(DragSource)

//------------------------------------------------------------------------
ClassificationProxy::ClassificationProxy(ModelAdapterSPtr model,
                                         GUI::View::ViewState &viewState,
                                         QObject* parent)
: QAbstractProxyModel{parent}
, m_viewState        (viewState)
, m_classification   {new ClassificationAdapter()}
{
  setSourceModel(model);
}

//------------------------------------------------------------------------
ClassificationProxy::~ClassificationProxy()
{
//   qDebug() << "Destroying classification Proxy";
}

//------------------------------------------------------------------------
void ClassificationProxy::setSourceModel(ModelAdapterSPtr sourceModel)
{
  if (m_model)
  {
    disconnect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(m_model.get(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    disconnect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model.get(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model.get(), SIGNAL(modelAboutToBeReset()),
               this, SLOT(sourceModelReset()));
  }

  m_numCategories.clear();
  m_categorySegmentations.clear();
  m_categoryVisibility.clear();

  m_model = sourceModel;

  if (m_model)
  {
    connect(m_model.get(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    connect(m_model.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_model.get(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(m_model.get(), SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model.get(), SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model.get(), SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelReset()));

    QAbstractProxyModel::setSourceModel(m_model.get());

    sourceRowsInserted(m_model->classificationRoot(), 0, m_model->rowCount(m_model->classificationRoot()) - 1);
    sourceRowsInserted(m_model->segmentationRoot()  , 0, m_model->rowCount(m_model->segmentationRoot()) - 1);
  }
}

//------------------------------------------------------------------------
QVariant ClassificationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid()) return QVariant();

  auto item = itemAdapter(proxyIndex);

  if (isSegmentation(item)) return item->data(role);

  if (isCategory(item))
  {
    auto category = toProxyPtr(toCategoryAdapterPtr(item));

    if (Qt::DisplayRole == role)
    {
      const int totalSegs = numSegmentations(category, true);

      return item->data(role).toString() + categorySuffix(totalSegs);
    }
    else if (Qt::CheckStateRole == role)
    {
      return m_categoryVisibility[category];
    }
    else
    {
      return item->data(role);
    }
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool ClassificationProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;

  if (index.isValid())
  {
    if (Qt::CheckStateRole == role)
    {
      const auto visibility = value.toBool();

      changeIndexVisibility(index, visibility);

      changeParentCheckStateRole(index, visibility);

      notifyModifiedRepresentations(index);

      result = true;
    }
    else
    {
      result = m_model->setData(mapToSource(index), value, role);
    }
  }

  return result;
}

//------------------------------------------------------------------------
bool ClassificationProxy::hasChildren(const QModelIndex& parent) const
{
  return hasValidIndexes() && (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

//------------------------------------------------------------------------
int ClassificationProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) return m_rootCategories.size();

  // Cast to base type
  auto parentItem = itemAdapter(parent);
  int rows = 0;

  if (isCategory(parentItem))
  {
    auto sourceCategory = toCategoryAdapterPtr(parentItem);
    auto proxyCategory  = toProxyPtr(sourceCategory);
    rows = numSubCategories(proxyCategory) + numSegmentations(proxyCategory);
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
  {
    return QModelIndex();
  }

  if (!parent.isValid())
  {
    return createSourceCategoryIndex(row, column, m_rootCategories.at(row));
  }

  auto parentItem = itemAdapter(parent);
  Q_ASSERT(isCategory(parentItem));
  auto parentSourceCategory = toCategoryAdapterPtr(parentItem);
  auto parentProxyCategory = toProxyPtr(parentSourceCategory);
  Q_ASSERT(parentProxyCategory);

  int categoryRows = numSubCategories(parentProxyCategory);

  if (row < categoryRows)
  {
    auto proxyCategory = parentProxyCategory->subCategories().at(row).get();
    return createSourceCategoryIndex(row, column, proxyCategory);
  }

  int segmentationRow = row - categoryRows;
  Q_ASSERT(segmentationRow < numSegmentations(parentProxyCategory));
  ItemAdapterPtr internalPtr{m_categorySegmentations[parentProxyCategory][segmentationRow]};

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid()) return QModelIndex();

  auto childItem = itemAdapter(child);

  QModelIndex parent;
  switch (childItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      auto childCategory        = toCategoryAdapterPtr(childItem);
      auto sourceParentCategory = childCategory->parent();
      auto proxyParentCategory  = toProxyPtr(sourceParentCategory);

      parent = categoryIndex(proxyParentCategory);
      break;
    }
    case ItemAdapter::Type::SEGMENTATION:
    {
      for(auto category : m_categorySegmentations.keys())
      {
        if (m_categorySegmentations[category].contains(childItem))
        {
          parent = categoryIndex(category);
          break;
        }
      }
      break;
    }
    default:
      Q_ASSERT(false);
      break;
  }

  return parent;
}


//------------------------------------------------------------------------
QModelIndex ClassificationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid()                       ||
      !hasValidIndexes()                           ||
      sourceIndex == m_model->classificationRoot() ||
      sourceIndex == m_model->sampleRoot()         ||
      sourceIndex == m_model->channelRoot()        ||
      sourceIndex == m_model->segmentationRoot())
  {
    return QModelIndex();
  }

  auto sourceItem = itemAdapter(sourceIndex);

  QModelIndex proxyIndex;
  switch(sourceItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      //Categories are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case ItemAdapter::Type::SEGMENTATION:
    {
      if (!m_categorySegmentations.isEmpty())
      {
        auto segmentation = segmentationPtr(sourceItem);
        Q_ASSERT(segmentation);
        auto proxyCategory = toProxyPtr(segmentation->category().get());
        if (proxyCategory)
        {
          int row = m_categorySegmentations[proxyCategory].indexOf(segmentation);
          if (row >= 0)
          {
            row += numSubCategories(proxyCategory);
            proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
          }
        }
      }
      break;
    }
    default:
      proxyIndex = QModelIndex();
      break;
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid()) return QModelIndex();

  auto proxyItem = itemAdapter(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      auto sourceCategory  = toCategoryAdapterPtr(proxyItem);

      sourceIndex = m_model->categoryIndex(sourceCategory);

      break;
    }
    case ItemAdapter::Type::SEGMENTATION:
    {
      auto proxySegmentation = segmentationPtr(proxyItem);
      Q_ASSERT(proxySegmentation);
      sourceIndex = m_model->segmentationIndex(proxySegmentation);
      break;
    }
    default:
      Q_ASSERT(false);
      break;
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
Qt::ItemFlags ClassificationProxy::flags(const QModelIndex& index) const
{
  auto indexFlags = QAbstractProxyModel::flags(index) | Qt::ItemIsDropEnabled;

  if (index.isValid())
  {
    auto sourceItem = itemAdapter(index);
    if (isCategory(sourceItem))
    {
      indexFlags = indexFlags | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    }
    else if (isSegmentation(sourceItem))
    {
      indexFlags = indexFlags | Qt::ItemIsDragEnabled;
    }
  }

  return indexFlags;
}


//------------------------------------------------------------------------
bool ClassificationProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  using  DraggedItem = QMap<int, QVariant>;

  auto root       = m_model->classification()->root().get();
  auto parentItem = parent.isValid() ? itemAdapter(parent) : root;

  DragSource source = NoSource;

  // Recover dragged item information
  QByteArray  encoded = data->data("application/x-qabstractitemmodeldatalist");

  QDataStream        stream(&encoded, QIODevice::ReadOnly);
  QList<DraggedItem> draggedItems;

  while (!stream.atEnd())
  {
    int row, col;
    DraggedItem itemData;
    stream >> row >> col >> itemData;

    switch (ItemAdapter::type(itemData[TypeRole].toInt()))
    {
      case ItemAdapter::Type::SEGMENTATION:
        source |= SegmentationSource;
        break;
      case ItemAdapter::Type::CATEGORY:
        source |= CategorySource;
        break;
      default:
        source = InvalidSource;
        break;
    }

    draggedItems << itemData;
  }

  // Change Category
  if (SegmentationSource == source && parentItem != root)
  {
    SegmentationAdapterList sources;
    for(auto draggedItem : draggedItems)
    {
      auto item = reinterpret_cast<ItemAdapterPtr>(draggedItem[RawPointerRole].value<quintptr>());
      sources << segmentationPtr(item);
    }

    CategoryAdapterPtr newCategory = nullptr;

    if (isCategory(parentItem))
    {
      newCategory = toCategoryAdapterPtr(parentItem);
    }
    else if (isSegmentation(parentItem))
    {
      auto segmentation = segmentationPtr(parentItem);

      newCategory = segmentation->category().get();
    }
    Q_ASSERT(newCategory);

    emit segmentationsDropped(sources, newCategory);
  }

  // Change category parent
  else if (CategorySource == source && isCategory(parentItem))
  {
    CategoryAdapterList sources;
    for(DraggedItem draggedItem : draggedItems)
    {
      auto item = reinterpret_cast<ItemAdapterPtr>(draggedItem[RawPointerRole].value<quintptr>());

      sources << toCategoryAdapterPtr(item);
    }
    emit categoriesDropped(sources, toCategoryAdapterPtr(parentItem));
  }
  else if (InvalidSource == source)
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------
int ClassificationProxy::numSegmentations(QModelIndex categoryIndex, bool recursive) const
{
  int total = 0;

  if(categoryIndex.isValid())
  {
    ItemAdapterPtr item = itemAdapter(categoryIndex);

    if (isCategory(item))
    {
      auto category = toCategoryAdapterPtr(item);

      total = numSegmentations(category);

      if (recursive)
      {
        for (int i = 0; i < numSubCategories(category); ++i)
        {
          total += numSegmentations(index(i, 0, categoryIndex), true);
        }
      }
    }
  }

  return total;
}

//------------------------------------------------------------------------
int ClassificationProxy::numSubCategories(QModelIndex categoryIndex) const
{
  if(!categoryIndex.isValid()) return 0;

  ItemAdapterPtr item = itemAdapter(categoryIndex);

  if (!isCategory(item)) return 0;

  auto category = toCategoryAdapterPtr(item);

  return numSubCategories(category);
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::segmentations(QModelIndex catIndex, bool recursive) const
{
  QModelIndexList segs;

  if(catIndex.isValid())
  {
    int start = numSubCategories(catIndex);
    int end = start + numSegmentations(catIndex) - 1;

    if (recursive)
    {
      for (int tax = 0; tax < start; tax++)
      {
        segs << segmentations(index(tax, 0, catIndex), true);
      }
    }

    if (start <= end)
    {
      segs << proxyIndices(catIndex, start, end);
    }
  }
  return segs;
}

//------------------------------------------------------------------------
SegmentationAdapterList ClassificationProxy::segmentations(CategoryAdapterPtr category, bool recursive) const
{
  SegmentationAdapterList categorySegmentation;

  if(category)
  {
    if (recursive)
    {
      for(auto subCategory : category->subCategories())
      {
        categorySegmentation << segmentations(subCategory.get(), recursive);
      }
    }

    for(auto item : m_categorySegmentations[category])
    {
      categorySegmentation << segmentationPtr(item);
    }
  }

  return categorySegmentation;
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::sourceIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;

  if(parent.isValid() && (start <= end))
  {
    for (int row = start; row <= end; row++)
    {
      QModelIndex sourceIndex = m_model->index(row, 0, parent);
      res << sourceIndex;

      int numChildren = m_model->rowCount(sourceIndex);
      if (numChildren > 0)
      {
        res << sourceIndices(sourceIndex,0,numChildren - 1);
      }
    }
  }

  return res;
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;

  if(parent.isValid() && (start <= end))
  {
    for (int row = start; row <= end; row++)
    {
      QModelIndex proxyIndex = index(row, 0, parent);
      res << proxyIndex;

      int numChildren = rowCount(proxyIndex);
      if (numChildren > 0)
      {
        res << proxyIndices(proxyIndex,0,numChildren - 1);
      }
    }
  }

  return res;
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if ((end < start)                           ||
      sourceParent == m_model->sampleRoot()   ||
      sourceParent == m_model->channelRoot())
  {
    return;
  }

  if (sourceParent == m_model->classificationRoot())
  {
    beginInsertRows(QModelIndex(), start, end);

    // We need to keep a copy of the original structure so we can delete it in the future
    // and provide valid indices
    CategoryAdapterList sourceCategories;

    for (int row = start; row <= end; row++)
    {
      auto insertedItem   = itemAdapter(m_model->index(row, 0, sourceParent));
      auto sourceCategory = toCategoryAdapterPtr(insertedItem);

      sourceCategories << sourceCategory;
    }

    int numRootCategories = end - start + 1;

    while (!sourceCategories.isEmpty())
    {
      auto sourceCategory = sourceCategories.takeFirst();
      auto proxyCategory  = createProxyCategory(sourceCategory);

      if (numRootCategories > 0)
      {
        m_rootCategories << proxyCategory;
        --numRootCategories;
      }

      for (auto subCategory : sourceCategory->subCategories())
      {
        sourceCategories << subCategory.get();
      }
    }

    for (auto rootCategory : m_rootCategories)
    {
      addCategory(rootCategory);
    }

    endInsertRows();
  }
  else if (sourceParent == m_model->segmentationRoot())
  {
    auto relations = groupSegmentationsByCategory(start, end);

    for(auto proxyCategory : relations.keys())
    {
      int numCats = numSubCategories(proxyCategory);
      int numSegs = numSegmentations(proxyCategory);

      int startRow = numCats  + numSegs;
      int endRow   = startRow + relations[proxyCategory].size() - 1;

      beginInsertRows(categoryIndex(proxyCategory), startRow, endRow);
      m_categorySegmentations[proxyCategory] << relations[proxyCategory];
      endInsertRows();
    }
  }
  else
  {
    ItemAdapterPtr parentItem = itemAdapter(sourceParent);
    if (isCategory(parentItem))
    {
      beginInsertRows(mapFromSource(sourceParent), start, end);
      for (int row = start; row <= end; row++)
      {
        auto sourceItem           = itemAdapter(m_model->index(row, 0, sourceParent));
        auto sourceCategory       = toCategoryAdapterPtr(sourceItem);
        auto sourceParentCategory = sourceCategory->parent();

        auto proxyParentCategory  = toProxyPtr(sourceParentCategory);
        auto proxyCategory        = createProxyCategory(sourceCategory);

        if (sourceParentCategory != m_model->classification()->root().get())
        {
          m_numCategories[proxyParentCategory] += 1;
        }
        m_numCategories[proxyCategory]      = sourceCategory->subCategories().size();
        m_categoryVisibility[proxyCategory] = Qt::Checked;
      }
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid()                ||
      sourceParent == m_model->sampleRoot()  ||
      sourceParent == m_model->channelRoot())
  {
    return;
  }

  if (sourceParent == m_model->segmentationRoot())
  {
    auto relations = groupSegmentationsByCategory(start, end);

    for(auto proxyCategory : relations.keys())
    {
      auto firstSeg = relations[proxyCategory].first();

      const unsigned NUM_SEGS = relations[proxyCategory].size();
      const unsigned SEG_POS  = m_categorySegmentations[proxyCategory].indexOf(firstSeg);

      // As rows to be removed are packed on contiguous positions and this proxy
      // keeps its original ordering, we can assume grouped segmentations are
      // also contiguous
      int startRow  = numSubCategories(proxyCategory) + SEG_POS;
      int endRow    = startRow + NUM_SEGS - 1;

      beginRemoveRows(categoryIndex(proxyCategory), startRow, endRow);
      for (unsigned i = 0; i < NUM_SEGS; ++i)
      {
        m_categorySegmentations[proxyCategory].removeAt(SEG_POS);
      }
      endRemoveRows();
    }
  }
  else // Handles classificationRoot and categoryNodes
  {
    auto proxyParent = mapFromSource(sourceParent);

    beginRemoveRows(proxyParent, start, end);
    for (int row=start; row <= end; row++)
    {
      auto item           = itemAdapter(m_model->index(row, 0, sourceParent));
      auto sourceCategory = toCategoryAdapterPtr(item);
      auto proxyCategory  = toProxyPtr(sourceCategory);
      removeCategory(proxyCategory);
    }
    endRemoveRows();
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                                   const QModelIndex &destinationParent, int destinationRow)
{
  Q_ASSERT(sourceStart == sourceEnd);

  QModelIndex proxySourceParent       = mapFromSource(sourceParent);
  QModelIndex proxyDestionationParent = mapFromSource(destinationParent);

  beginMoveRows(proxySourceParent, sourceStart, sourceEnd,
                proxyDestionationParent, destinationRow);

  auto movingItem     = itemAdapter(sourceParent.child(sourceStart, 0));
  auto movingCategory = toProxySPtr(toCategoryAdapterPtr(movingItem));

  if (proxySourceParent.isValid())
  {
    auto proxySourceItem     = itemAdapter(proxySourceParent);
    auto proxySourceCategory = toProxyPtr(toCategoryAdapterPtr(proxySourceItem));

    m_numCategories[proxySourceCategory] -=1;
  }
  else
  {
    m_rootCategories.removeOne(movingCategory.get());
  }

  CategoryAdapterPtr proxyDestinationCategory = nullptr;
  if (proxyDestionationParent.isValid())
  {
    auto proxyDestinationItem = itemAdapter(destinationParent);
    proxyDestinationCategory  = toProxyPtr(toCategoryAdapterPtr(proxyDestinationItem));

    m_numCategories[proxyDestinationCategory] +=1;
  }
  else
  {
    proxyDestinationCategory = m_classification->root().get();

    m_rootCategories << movingCategory.get();
  }

  proxyDestinationCategory->addSubCategory(movingCategory);
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
  endMoveRows();
}

//------------------------------------------------------------------------
bool ClassificationProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
{
  if(!topLeft.isValid() || !bottomRight.isValid()) return false;

  result << topLeft;

  if (topLeft == bottomRight) return true;

  for (int r = 0; r < m_model->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result)) return true;
  }

  for (int r = topLeft.row() + 1; r < m_model->rowCount(topLeft.parent()); r++)
  {
    if (indices(topLeft.sibling(r,0), bottomRight, result)) return true;
  }

  return false;
}

//------------------------------------------------------------------------
SegmentationAdapterPtr ClassificationProxy::findSegmentation(QString tooltip)
{
  for(auto category : m_categorySegmentations.keys())
  {
    for(auto segmentation : m_categorySegmentations[category])
    {
      if (segmentation->data(Qt::ToolTipRole) == tooltip)
      {
        return segmentationPtr(segmentation);
      }
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  if(!sourceTopLeft.isValid() || !sourceBottomRight.isValid()) return;

  QModelIndexList sources;

  // Heterogeneous data changed
  if (sourceTopLeft.parent() != sourceBottomRight.parent() &&
    !(isCategoryIndex(sourceTopLeft) && isCategoryIndex(sourceBottomRight)))
  {

  }
  else if (sourceTopLeft.parent() == m_model->segmentationRoot())
  {
    auto itemsByCurrentCategory = groupSegmentationsByCategory(sourceTopLeft.row(), sourceBottomRight.row());

    for(auto currentCategory : itemsByCurrentCategory.keys())
    {
      auto proxyDestination  = categoryIndex(currentCategory);
      auto groupedItems      = groupSegmentationsByPreviousCategory(itemsByCurrentCategory[currentCategory], currentCategory);

      auto itemsByPreviousCategory = groupedItems.first;
      auto unchangedItems          = groupedItems.second;

      processConsecutiveDataChanges(unchangedItems, currentCategory);

      for (auto prevCategory : itemsByPreviousCategory.keys())
      {
        auto proxySource = categoryIndex(prevCategory);

        processConsecutiveCategoryChanges(itemsByPreviousCategory[prevCategory], prevCategory, currentCategory, proxySource, proxyDestination);
      }
    }
  }
  else if (isCategoryIndex(sourceTopLeft))
  {
    indices(sourceTopLeft, sourceBottomRight, sources);

    for (auto sourceItem : sources)
    {
      auto sourceCategory = toCategoryAdapterPtr(sourceItem);
      auto proxyCategory  = toProxyPtr(sourceCategory);

      proxyCategory->setName (sourceCategory->name());
      proxyCategory->setColor(sourceCategory->color());
    }

    emit dataChanged(sourceTopLeft, sourceBottomRight);
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_classification = std::make_shared<ClassificationAdapter>();
    m_sourceCategory.clear();
    m_rootCategories.clear();
    m_numCategories.clear();
    m_categorySegmentations.clear();
    m_categoryVisibility.clear();
  }
  endResetModel();
}

//------------------------------------------------------------------------
bool idOrdered(SegmentationAdapterPtr seg1, SegmentationAdapterPtr seg2)
{
  return seg1 && seg2 && (seg1->number() < seg2->number());
}

//------------------------------------------------------------------------
void ClassificationProxy::addCategory(CategoryAdapterPtr category)
{
  if(category)
  {
    m_numCategories[category]      = category->subCategories().size();
    m_categoryVisibility[category] = Qt::Checked;

    for(auto subCategory : category->subCategories())
    {
      addCategory(subCategory.get());
    }
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::removeCategory(CategoryAdapterPtr proxyCategory)
{
  if(!proxyCategory) return;

  // First remove its subCategories
  for(auto subCategory : proxyCategory->subCategories())
  {
    removeCategory(subCategory.get());
  }

  m_rootCategories       .removeOne(proxyCategory); //Safe even if it's not root category
  m_numCategories        .remove(proxyCategory);
  m_categorySegmentations.remove(proxyCategory);
  m_sourceCategory       .remove(proxyCategory);

  auto parentNode = proxyCategory->parent();
  if (parentNode && parentNode != m_classification->root().get())
  {
    m_numCategories[parentNode] -= 1;
  }

  auto sptr = m_classification->category(proxyCategory->classificationName());
  m_classification->removeCategory(sptr);
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::categoryIndex(CategoryAdapterPtr proxyCategory) const
{
  if (proxyCategory && proxyCategory->parent())
  {
    int row = 0;

    for (auto subCategory : proxyCategory->parent()->subCategories())
    {
      if (subCategory.get() == proxyCategory)
      {
        return createSourceCategoryIndex(row, 0, proxyCategory);
      }
      ++row;
    }
  }

  return QModelIndex();
}

//------------------------------------------------------------------------
ClassificationProxy::CategorySegmentations ClassificationProxy::groupSegmentationsByCategory(int start, int end)
{
  CategorySegmentations relations;

  if(start <= end)
  {
    for (int row=start; row <= end; row++)
    {
      auto sourceIndex    = m_model->index(row, 0, m_model->segmentationRoot());
      auto sourceItem     = itemAdapter(sourceIndex);
      auto segmentation   = segmentationPtr(sourceItem);
      auto sourceCategory = segmentation->category().get();

      if (sourceCategory)
      {
        //qDebug() << "Source Category: " << sourceCategory->name();
        auto proxyCategory = toProxyPtr(sourceCategory);
        //qDebug() << "Proxy Category: " << proxyCategory->name();
        relations[proxyCategory] << sourceItem;
      }
    }
  }

  return relations;
}

//------------------------------------------------------------------------
ClassificationProxy::SegmentationsGroup ClassificationProxy::groupSegmentationsByPreviousCategory(ItemAdapterList    sourceItems,
                                                                                                  CategoryAdapterPtr currentCategory)
{
  CategorySegmentations prevCategories;
  ItemAdapterList       unchangedItems;

  for (auto sourceItem : sourceItems)
  {
    for(auto &prevCategory : m_categorySegmentations.keys())
    {
      if (m_categorySegmentations[prevCategory].contains(sourceItem))
      {
        if (prevCategory != currentCategory)
        {
          prevCategories[prevCategory] << sourceItem;
        }
        else
        {
          unchangedItems << sourceItem;
        }
        break;
      }
    }
  }

  return SegmentationsGroup(prevCategories, unchangedItems);
}

//------------------------------------------------------------------------
void ClassificationProxy::processConsecutiveCategoryChanges(ItemAdapterList    sourceItems,
                                                            CategoryAdapterPtr prevCategory,
                                                            CategoryAdapterPtr currentCategory,
                                                            const QModelIndex& proxySource,
                                                            const QModelIndex& proxyDestination)
{
  int start;
  int end;
  int next;
  ItemAdapterList consecutiveItems;

  for (int i = 0; i <= sourceItems.size(); ++i)
  {
    if (i < sourceItems.size())
    {
      next = currentSegmentationRow(sourceItems[i], prevCategory);

      if (consecutiveItems.isEmpty())
      {
        start = next;
        end   = next;
      }
    }

    if (next - end > 1 || i == sourceItems.size())
    {
      if (!consecutiveItems.isEmpty())
      {
        int newRow = rowCount(proxyDestination);

        beginMoveRows(proxySource, start, end, proxyDestination, newRow);
        for (auto sourceItem : sourceItems)
        {
          m_categorySegmentations[prevCategory].removeOne(sourceItem);
          m_categorySegmentations[currentCategory] << sourceItem;
        }
        endMoveRows();

        consecutiveItems.clear();
        start = next;
      }
    }

    if (i < sourceItems.size())
    {
      auto sourceItem = sourceItems[i];
      consecutiveItems << sourceItem;
      next = currentSegmentationRow(sourceItem, prevCategory);
    }
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::processConsecutiveDataChanges(ItemAdapterList    sourceItems,
                                                        CategoryAdapterPtr currentCategory)
{
  int end;
  int next;
  ItemAdapterList consecutiveItems;

  for (int i = 0; i <= sourceItems.size(); ++i)
  {
    if (i < sourceItems.size())
    {
      next = currentSegmentationRow(sourceItems[i], currentCategory);

      if (consecutiveItems.isEmpty())
      {
        end = next;
      }
    }

    if (next - end > 1 || i == sourceItems.size())
    {
      if (!consecutiveItems.isEmpty())
      {
        auto index       = categoryIndex(currentCategory);
        auto topLeft     = index.child(currentSegmentationRow(consecutiveItems.first(), currentCategory), 0);
        auto bottomRight = index.child(currentSegmentationRow(consecutiveItems.last() , currentCategory), 0);

        emit dataChanged(topLeft, bottomRight);

        consecutiveItems.clear();
      }
    }

    if (i < sourceItems.size())
    {
      auto sourceItem = sourceItems[i];
      consecutiveItems << sourceItem;
      next = currentSegmentationRow(sourceItem, currentCategory);
    }
  }
}

//------------------------------------------------------------------------
int ClassificationProxy::currentSegmentationRow(ItemAdapterPtr sourceItem, CategoryAdapterPtr category)
{
  return numSubCategories(category) + m_categorySegmentations[category].indexOf(sourceItem);
}

//------------------------------------------------------------------------
CategoryAdapterPtr ClassificationProxy::createProxyCategory(CategoryAdapterPtr sourceCategory)
{
  auto proxyCategory  = m_classification->createCategory(sourceCategory->classificationName()).get();

  proxyCategory->setColor(sourceCategory->color());
  m_sourceCategory[proxyCategory] = sourceCategory;

  return proxyCategory;
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::createSourceCategoryIndex(int row, int column, CategoryAdapterPtr category) const
{
  ItemAdapterPtr sourceCategory = toSourcePtr(category);

  return createIndex(row, column, sourceCategory);
}

//------------------------------------------------------------------------
CategoryAdapterPtr ClassificationProxy::toProxyPtr(CategoryAdapterPtr sourceCategory) const
{
  return toProxySPtr(sourceCategory).get();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationProxy::toProxySPtr(CategoryAdapterPtr sourceCategory) const
{
  CategoryAdapterSPtr proxyCategory;

  if (sourceCategory)
  {
    if (sourceCategory->parent())
    {
      auto proxyCategoryPtr = m_sourceCategory.key(sourceCategory);

      proxyCategory = proxyCategoryPtr->parent()->subCategory(proxyCategoryPtr->name());
    }
    else
    {
      proxyCategory = m_classification->root();
    }
  }

  return proxyCategory;
}

//------------------------------------------------------------------------
CategoryAdapterPtr ClassificationProxy::toSourcePtr(CategoryAdapterPtr proxyCategory) const
{
  return toSourceSPtr(proxyCategory).get();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr ClassificationProxy::toSourceSPtr(CategoryAdapterPtr proxyCategory) const
{
  Q_ASSERT(proxyCategory);

  auto sourceCategory = m_sourceCategory[proxyCategory];

  return m_model->classification()->category(sourceCategory->classificationName());
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ClassificationProxy::toSourceSPtr(SegmentationAdapterPtr segmentation) const
{
  Q_ASSERT(segmentation);

  return m_model->smartPointer(segmentation);
}


//------------------------------------------------------------------------
bool ClassificationProxy::isCategoryIndex(const QModelIndex &index) const
{
  return !index.parent().isValid()                        ||
          index.parent() == m_model->classificationRoot() ||
          (index.parent() != m_model->sampleRoot() && index.parent() != m_model->channelRoot() && index.parent() != m_model->segmentationRoot());
}

//------------------------------------------------------------------------
int ClassificationProxy::numSegmentations(CategoryAdapterPtr proxyCategory, bool recursive) const
{
  int total = m_categorySegmentations[proxyCategory].size();

  if (recursive)
  {
    for(auto subCategory : proxyCategory->subCategories())
    {
      total += numSegmentations(subCategory.get(), recursive);
    }
  }

  return total;
}

//------------------------------------------------------------------------
ViewItemAdapterList ClassificationProxy::childrenSegmentations(CategoryAdapterPtr proxyCategory, bool recursive) const
{
  ViewItemAdapterList children;

  for (auto item : m_categorySegmentations[proxyCategory])
  {
    children << segmentationPtr(item);
  }

  if (recursive)
  {
    for(auto subCategory : proxyCategory->subCategories())
    {
      children << childrenSegmentations(subCategory.get(), recursive);
    }
  }

  return children;
}

//------------------------------------------------------------------------
int ClassificationProxy::numSubCategories(CategoryAdapterPtr proxyCategory) const
{
  // We can't rely on source's model to deal with row counting
  return m_numCategories[proxyCategory];
}

//------------------------------------------------------------------------
QString ClassificationProxy::categorySuffix(const int numSegmentations) const
{
  return numSegmentations>0?QString(" (%1)").arg(numSegmentations):"";
}

//------------------------------------------------------------------------
void ClassificationProxy::changeIndexVisibility(const QModelIndex &index, bool value)
{
  auto item = itemAdapter(index);

  if (isCategory(item))
  {
    auto category = toProxyPtr(toCategoryAdapterPtr(item));

    changeCategoryVisibility(category, value);

    changeChildrenVisibility(index, value);
  }
  else if (isSegmentation(item))
  {
    m_model->setData(mapToSource(index), value, Qt::CheckStateRole);
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::changeCategoryVisibility(CategoryAdapterPtr category, bool value)
{
  m_categoryVisibility[category] = value?Qt::Checked:Qt::Unchecked;
}

//------------------------------------------------------------------------
void ClassificationProxy::changeChildrenVisibility(const QModelIndex &index, bool value)
{
  for (int r=0; r < rowCount(index); r++)
  {
    changeIndexVisibility(index.child(r,0), value);
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::changeParentCheckStateRole(const QModelIndex &index, bool value)
{
  auto parentIndex = parent(index);

  while (parentIndex.isValid())
  {
    if (parentIndex.isValid())
    {
      auto parentItem          = itemAdapter(parentIndex);
      auto proxyParentCategory = toProxyPtr(toCategoryAdapterPtr(parentItem));

      int  parentRows = rowCount(parentIndex);
      auto checkState = Qt::Unchecked;

      for(int row = 0; row < parentRows; row++)
      {
        auto rowState = parentIndex.child(row, 0).data(Qt::CheckStateRole).toBool()?Qt::Checked:Qt::Unchecked;
        if (0 == row)
        {
          checkState = rowState;
        }
        else if (checkState != rowState)
        {
          checkState = Qt::PartiallyChecked;
          break;
        }
      }

      m_categoryVisibility[proxyParentCategory] = checkState;
    }

    parentIndex = parent(parentIndex);
  }

  emit dataChanged(parentIndex, parentIndex);
}

//------------------------------------------------------------------------
void ClassificationProxy::notifyModifiedRepresentations(const QModelIndex &index)
{
  auto item = itemAdapter(index);

  ViewItemAdapterList modifiedItems;

  if (isCategory(item))
  {
    auto category = toProxyPtr(toCategoryAdapterPtr(item));
    modifiedItems = childrenSegmentations(category, true);
  }
  else if (isSegmentation(item))
  {
    modifiedItems << segmentationPtr(item);
  }
  else
  {
    Q_ASSERT(false);
  }

  m_viewState.invalidateRepresentations(modifiedItems);
}

//------------------------------------------------------------------------
bool ClassificationProxy::hasValidIndexes() const
{
  return !m_rootCategories.isEmpty();
}
