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


#include "ClassificationProxy.h"

#include <QMimeData>
#include <QPixmap>
#include <QPainter>

using namespace EspINA;

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
ClassificationProxy::ClassificationProxy(ModelAdapterSPtr model, QObject* parent)
: QAbstractProxyModel(parent)
{
  setSourceModel(model);
}

//------------------------------------------------------------------------
ClassificationProxy::~ClassificationProxy()
{
//   qDebug() << "Destroying Category Proxy";
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
  }

  QAbstractProxyModel::setSourceModel(m_model.get());

  sourceRowsInserted(m_model->classificationRoot(), 0, m_model->rowCount(m_model->classificationRoot())     - 1);
  sourceRowsInserted(m_model->segmentationRoot()  , 0, m_model->rowCount(m_model->segmentationRoot()) - 1);
}

//------------------------------------------------------------------------
QVariant ClassificationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  auto item = itemAdapter(proxyIndex);
  switch (item->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      auto category = categoryPtr(item);
      if (Qt::DisplayRole == role)
      {
        //int numSegs   = numSegmentations(category, false);
        int totalSegs = numSegmentations(category, true);
        QString suffix;
        if (totalSegs > 0)
          suffix += QString(" (%1)").arg(totalSegs);

        return item->data(role).toString() + suffix;
      }
      else if (Qt::CheckStateRole == role)
        return m_categoryVisibility[category];
      else
        return item->data(role);
    }
    case ItemAdapter::Type::SEGMENTATION:
        return item->data(role);

    default:
      Q_ASSERT(false);
      break;
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool ClassificationProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;

  if (index.isValid())
  {
    ItemAdapterPtr item = itemAdapter(index);
    if (Qt::CheckStateRole == role)
    {
      if (isCategory(item))
      {
        CategoryAdapterPtr category = categoryPtr(item);
        m_categoryVisibility[category] = value.toBool()?Qt::Checked:Qt::Unchecked;

        int rows = rowCount(index);
        for (int r=0; r<rows; r++)
        {
          setData(index.child(r,0), value, role);
        }
        emit dataChanged(index, index);

        result = true;
      } else if (isSegmentation(item))
      {
        result = m_model->setData(mapToSource(index), value, role);
      }

      QModelIndex parentIndex = parent(index);
      if (parentIndex.isValid())
      {
        ItemAdapterPtr parentItem = itemAdapter(parentIndex);
        CategoryAdapterPtr parentCategory = categoryPtr(parentItem);
        int parentRows = rowCount(parentIndex);
        Qt::CheckState checkState = Qt::Unchecked;
        for(int r=0; r < parentRows; r++)
        {
          Qt::CheckState rowState = parentIndex.child(r, 0).data(Qt::CheckStateRole).toBool()?Qt::Checked:Qt::Unchecked;
          if (0 == r)
            checkState = rowState;
          else
            if (checkState != rowState)
            {
              checkState = Qt::PartiallyChecked;
              break;
            }
        }
        m_categoryVisibility[parentCategory] = checkState;
        emit dataChanged(parentIndex, parentIndex);
      }
    }
    else
      result = m_model->setData(mapToSource(index), value, role);
  }

  return result;
}

//------------------------------------------------------------------------
bool ClassificationProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}


//------------------------------------------------------------------------
int ClassificationProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_rootCategories.size();

  // Cast to base type
  auto parentItem = itemAdapter(parent);
  int rows = 0;

  if (isCategory(parentItem))
  {
    auto category = categoryPtr(parentItem);
    rows = numSubCategories(category) + numSegmentations(category);
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return mapFromSource(m_model->index(row,column,m_model->classificationRoot()));

  auto parentItem = itemAdapter(parent);
  Q_ASSERT(isCategory(parentItem));
  auto parentCategory = categoryPtr(parentItem);
  Q_ASSERT(parentCategory);

  int categoryRows = numSubCategories(parentCategory);
  if (row < categoryRows)
  {
    auto parentCategoryIndex = m_model->categoryIndex(parentCategory);
    auto index = m_model->index(row, column, parentCategoryIndex);

    return mapFromSource(index);
  }

  int segmentationRow = row - categoryRows;
  Q_ASSERT(segmentationRow < numSegmentations(parentCategory));
  ItemAdapterPtr internalPtr{m_categorySegmentations[parentCategory][segmentationRow]};

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  auto childItem = itemAdapter(child);

  QModelIndex parent;
  switch (childItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      auto childCategory = categoryPtr(childItem);
      auto sourceParent  = m_model->categoryIndex(childCategory->parent());
      parent = mapFromSource(sourceParent);
      break;
    }
    case ItemAdapter::Type::SEGMENTATION:
    {
      for(auto category : m_categorySegmentations.keys())
      {
        if (m_categorySegmentations[category].contains(childItem))
        {
          parent = mapFromSource(m_model->categoryIndex(category));
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
  if (!sourceIndex.isValid())
    return QModelIndex();

  if (sourceIndex == m_model->classificationRoot()
   || sourceIndex == m_model->sampleRoot()
   || sourceIndex == m_model->channelRoot()
   || sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  auto sourceItem = itemAdapter(sourceIndex);

  QModelIndex proxyIndex;
  switch(sourceItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      //Taxonomies are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case ItemAdapter::Type::SEGMENTATION:
    {
      auto segmentation = segmentationPtr(sourceItem);
      Q_ASSERT(segmentation);
      auto category = segmentation->category().get();
      if (category)
      {
        int row = m_categorySegmentations[category].indexOf(segmentation);
        if (row >= 0)
        {
          row += numSubCategories(category);
          proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
        }
      }
      break;
    }
    default:
      proxyIndex = QModelIndex();
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex ClassificationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  auto proxyItem = itemAdapter(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case ItemAdapter::Type::CATEGORY:
    {
      auto proxyCategory = categoryPtr(proxyItem);
      Q_ASSERT(proxyCategory);
      sourceIndex = m_model->categoryIndex(proxyCategory);
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
  Qt::ItemFlags f = QAbstractProxyModel::flags(index) | Qt::ItemIsDropEnabled;

  if (index.isValid())
  {
    ItemAdapterPtr sourceItem = itemAdapter(index);
    if (isCategory(sourceItem))
      f = f | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    else if (isSegmentation(sourceItem))
      f = f | Qt::ItemIsDragEnabled;
  }

  return f;
}


//------------------------------------------------------------------------
bool ClassificationProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  using  DraggedItem = QMap<int, QVariant>;

  CategoryAdapterPtr root = m_model->classification()->root().get();

  ItemAdapterPtr parentItem = parent.isValid()?itemAdapter(parent):root;

  DragSource source = NoSource;

  // Recover dragged item information
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

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
      newCategory = categoryPtr(parentItem);
    }
    else if (isSegmentation(parentItem))
    {
      auto segmentation = segmentationPtr(parentItem);
      newCategory = segmentation->category().get();
    }
    Q_ASSERT(newCategory);

    emit segmentationsDragged(sources, newCategory);
  }

  // Change category parent
  else if (CategorySource == source && isCategory(parentItem))
  {
    CategoryAdapterList sources;
    foreach(DraggedItem draggedItem , draggedItems)
    {
      ItemAdapterPtr item = reinterpret_cast<ItemAdapterPtr>(draggedItem[RawPointerRole].value<quintptr>());
      sources << categoryPtr(item);
    }
    emit categoriesDragged(sources, categoryPtr(parentItem));
  }
  else if (InvalidSource == source)
    return false;

  return true;
}

//------------------------------------------------------------------------
int ClassificationProxy::numSegmentations(QModelIndex taxIndex, bool recursive) const
{
  ItemAdapterPtr item = itemAdapter(taxIndex);
  if (!isCategory(item))
    return 0;

  auto category = categoryPtr(item);
  int total = numSegmentations(category);
  if (recursive)
  {
    int numTax = numSubCategories(category);
    for (int subTax = 0; subTax < numTax; subTax++)
    {
      total += numSegmentations(index(subTax, 0, taxIndex), true);
    }
  }
  return total;
}

//------------------------------------------------------------------------
int ClassificationProxy::numSubCategories(QModelIndex taxIndex) const
{
  ItemAdapterPtr item = itemAdapter(taxIndex);
  if (!isCategory(item))
    return 0;

  auto category = categoryPtr(item);

  return numSubCategories(category);
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::segmentations(QModelIndex taxIndex, bool recursive) const
{
  QModelIndexList segs;

  int start = numSubCategories(taxIndex);
  int end = start + numSegmentations(taxIndex) - 1;
  if (recursive)
  {
    for (int tax = 0; tax < start; tax++)
      segs << segmentations(index(tax, 0, taxIndex), true);
  }
  if (start <= end)
    segs << proxyIndices(taxIndex, start, end);

  return segs;
}

//------------------------------------------------------------------------
SegmentationAdapterList ClassificationProxy::segmentations(CategoryAdapterPtr category, bool recursive) const
{
  SegmentationAdapterList categorySegmentation;

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

  return categorySegmentation;
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::sourceIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;
//   static int indent = 0;

//   QString tab = std::string(indent*2,' ').c_str();
//   qDebug() <<  tab << parent.data(Qt::DisplayRole).toString() << m_model->rowCount(parent) << start << end;
  for (int row = start; row <= end; row++)
  {
    QModelIndex sourceIndex = m_model->index(row, 0, parent);
//     qDebug() << tab << "  " << sourceIndex.data(Qt::DisplayRole).toString();
    res << sourceIndex;

//     indent++;
    int numChildren = m_model->rowCount(sourceIndex);
    if (numChildren > 0)
      res << sourceIndices(sourceIndex,0,numChildren - 1);
//     indent--;
  }

  return res;
}

//------------------------------------------------------------------------
QModelIndexList ClassificationProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
{
  QModelIndexList res;
  for (int row = start; row <= end; row++)
  {
    QModelIndex proxyIndex = index(row, 0, parent);
    res << proxyIndex;

    int numChildren = rowCount(proxyIndex);
    if (numChildren > 0)
      res << proxyIndices(proxyIndex,0,numChildren - 1);
  }

  return res;
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot() ||
      sourceParent == m_model->channelRoot())
    return;

  if (sourceParent == m_model->classificationRoot())
  {
    beginInsertRows(QModelIndex(), start, end);
    for (int row = start; row <= end; row++)
    {
      ItemAdapterPtr       insertedItem     = itemAdapter(m_model->index(row, 0, sourceParent));
      CategoryAdapterPtr insertedElement = categoryPtr(insertedItem);

      m_rootCategories << insertedElement;

      addCategory(insertedElement);
    }
    endInsertRows();
  } else if (sourceParent == m_model->segmentationRoot())
  {
    QMap<CategoryAdapterPtr , QList<ItemAdapterPtr > > relations;

    for (int row=start; row <= end; row++)
    {
      auto sourceIndex  = m_model->index(row, 0, sourceParent);
      auto sourceItem   = itemAdapter(sourceIndex);
      auto segmentation = segmentationPtr(sourceItem);
      auto category     = segmentation->category().get();

      if (category)
        relations[category] << sourceItem;
    }

    for(auto category : relations.keys())
    {
      int numTaxs = numSubCategories(category);
      int numSegs = numSegmentations(category);

      int startRow = numTaxs + numSegs;
      int endRow   = startRow + relations[category].size() - 1;

      auto proxyCategory = mapFromSource(m_model->categoryIndex(category));

      beginInsertRows(proxyCategory, startRow, endRow);
      m_categorySegmentations[category] << relations[category];
      endInsertRows();
    }
  } else
  {
    ItemAdapterPtr parentItem = itemAdapter(sourceParent);
    if (isCategory(parentItem))
    {
      beginInsertRows(mapFromSource(sourceParent), start, end);
      for (int row = start; row <= end; row++)
      {
        auto sourceRow  = itemAdapter(m_model->index(row, 0, sourceParent));
        auto category   = categoryPtr(sourceRow);
        auto parentNode = category->parent();

        if (parentNode != m_model->classification()->root().get())
        {
          m_numCategories[parentNode] += 1;
        }
        m_numCategories[category]      = category->subCategories().size();
        m_categoryVisibility[category] = Qt::Checked;
      }
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot()
   || sourceParent == m_model->channelRoot())
    return;

  if (sourceParent == m_model->segmentationRoot())
  {
    for (int row=start; row <= end; row++)
    {
      auto sourceIndex  = m_model->index(row, 0, sourceParent);
      auto proxyIndex   = mapFromSource(sourceIndex);
      auto item         = itemAdapter(sourceIndex);
      auto segmentation = segmentationPtr(item);
      auto category     = segmentation->category().get();

      int segmentationRow = m_categorySegmentations[category].indexOf(item);
      if (segmentationRow >= 0)
      {
        int row = numSubCategories(category) + segmentationRow;

        beginRemoveRows(proxyIndex.parent(), row, row);
        m_categorySegmentations[category].removeAt(segmentationRow);
        endRemoveRows();
      }
    }
    return;
  }else // Handles classificationRoot and categoryNodes
  {
    beginRemoveRows(mapFromSource(sourceParent), start, end);
    for (int row=start; row <= end; row++)
    {
      auto item     = itemAdapter(m_model->index(row, 0, sourceParent));
      auto category = categoryPtr(item);
      removeCategory(category);
    }
    endRemoveRows();
    return;
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

  ItemAdapterPtr movingItem = itemAdapter(sourceParent.child(sourceStart, 0));
  CategoryAdapterPtr movingCategory = categoryPtr(movingItem);

  if (proxySourceParent.isValid())
  {
    ItemAdapterPtr sourceItem = itemAdapter(proxySourceParent);
    CategoryAdapterPtr sourceCategory = categoryPtr(sourceItem);
    m_numCategories[sourceCategory] -=1;
  } else
  {
    m_rootCategories.removeOne(movingCategory);
  }

  if (proxyDestionationParent.isValid())
  {
    ItemAdapterPtr destinationItem = itemAdapter(destinationParent);
    CategoryAdapterPtr destinationCategory = categoryPtr(destinationItem);

    m_numCategories[destinationCategory] +=1;
  } else
  {
    m_rootCategories << movingCategory;
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
  endMoveRows();
}


//------------------------------------------------------------------------
bool ClassificationProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
{
  result << topLeft;

  if (topLeft == bottomRight)
    return true;

  for (int r = 0; r < m_model->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result))
      return true;
  }

  for (int r = topLeft.row(); r < m_model->rowCount(topLeft.parent()); r++)
    if (indices(topLeft.sibling(r,0), bottomRight, result))
      return true;

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
  QModelIndexList sources;

  indices(sourceTopLeft, sourceBottomRight, sources);

  for(auto source : sources)
  {
    if (source.parent() == m_model->segmentationRoot())
    {
      bool indexChanged = true;
      auto sourceItem   = itemAdapter(source);
      auto segmentation = segmentationPtr(sourceItem);

      CategoryAdapterPtr prevCategory = nullptr;

      for(auto category : m_categorySegmentations.keys())
      {
        if (m_categorySegmentations[category].contains(sourceItem))
        {
          indexChanged = category != segmentation->category().get();
          prevCategory = category;
          break;
        }
      }

      if (prevCategory && indexChanged)
      {
        auto source      = m_model->categoryIndex(prevCategory);
        auto destination = m_model->categoryIndex(segmentation->category());

        auto proxySource      = mapFromSource(source);
        auto proxyDestination = mapFromSource(destination);

        int currentRow = numSubCategories(prevCategory) + m_categorySegmentations[prevCategory].indexOf(sourceItem);
        int newRow     = rowCount(proxyDestination);

        beginMoveRows(proxySource, currentRow, currentRow, proxyDestination, newRow);
        m_categorySegmentations[prevCategory].removeOne(sourceItem);
        m_categorySegmentations[segmentation->category().get()] << sourceItem;
        endMoveRows();
      }
    }

    auto proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::sourceModelReset()
{
  beginResetModel();
  {
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
  return seg1->number() < seg2->number();
}

//------------------------------------------------------------------------
void ClassificationProxy::addCategory(CategoryAdapterPtr category)
{
  m_numCategories[category]      = category->subCategories().size();
  m_categoryVisibility[category] = Qt::Checked;

  for(auto subCategory : category->subCategories())
  {
    addCategory(subCategory.get());
  }
}

//------------------------------------------------------------------------
void ClassificationProxy::removeCategory(CategoryAdapterPtr category)
{
  // First remove its subCategories
  for(auto subCategory : category->subCategories())
  {
    removeCategory(subCategory.get());
  }

  m_rootCategories.removeOne(category); //Safe even if it's not root category
  m_numCategories.remove(category);
  m_categorySegmentations.remove(category);

  auto parentNode = category->parent();
  if (parentNode != m_model->classification()->root().get())
  {
    m_numCategories[parentNode] -= 1;
  }
}

//------------------------------------------------------------------------
int ClassificationProxy::numSegmentations(CategoryAdapterPtr category, bool recursive) const
{
  int total = m_categorySegmentations[category].size();

  if (recursive)
    foreach(CategoryAdapterSPtr subtax, category->subCategories())
    {
      total += numSegmentations(subtax.get(), recursive);
    }

  return total;
}
//------------------------------------------------------------------------
int ClassificationProxy::numSubCategories(CategoryAdapterPtr category) const
{
  // We can't rely on source's model to deal with row counting
  return m_numCategories[category];
}
