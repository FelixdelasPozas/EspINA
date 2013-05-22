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


#include "TaxonomyProxy.h"

#include "Core/Model/Segmentation.h"
#include "Core/Model/EspinaModel.h"
#include <Core/Model/Taxonomy.h>

#include <QMimeData>
#include <QPixmap>
#include <QPainter>

using namespace EspINA;

typedef QSet<ModelItemPtr > SegSet;

enum DragSourceEnum {
  NoSource           = 0x0,
  SegmentationSource = 0x1,
  TaxonomySource     = 0x2,
  InvalidSource      = 0x3
};
Q_DECLARE_FLAGS(DragSource, DragSourceEnum);

Q_DECLARE_OPERATORS_FOR_FLAGS(DragSource)

//------------------------------------------------------------------------
TaxonomyProxy::TaxonomyProxy(QObject* parent)
: QAbstractProxyModel(parent)
, m_model(NULL)
{
}

//------------------------------------------------------------------------
TaxonomyProxy::~TaxonomyProxy()
{
//   qDebug() << "Destroying Taxonomy Proxy";
}

//------------------------------------------------------------------------
void TaxonomyProxy::setSourceModel(EspinaModel *sourceModel)
{
  if (m_model)
  {
    disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    disconnect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
               this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model, SIGNAL(modelAboutToBeReset()),
               this, SLOT(sourceModelReset()));
  }

  m_numTaxonomies.clear();
  m_taxonomySegmentations.clear();
  m_taxonomyVisibility.clear();

  m_model = sourceModel;

  if (m_model)
  {
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model, SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelReset()));
  }

  QAbstractProxyModel::setSourceModel(m_model);

  sourceRowsInserted(m_model->taxonomyRoot()    , 0, m_model->rowCount(m_model->taxonomyRoot())     - 1);
  sourceRowsInserted(m_model->segmentationRoot(), 0, m_model->rowCount(m_model->segmentationRoot()) - 1);
}

//------------------------------------------------------------------------
QVariant TaxonomyProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItemPtr item = indexPtr(proxyIndex);
  switch (item->type())
  {
    case EspINA::TAXONOMY:
    {
      TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
      if (Qt::DisplayRole == role)
      {
        //int numSegs   = numSegmentations(taxonomy, false);
        int totalSegs = numSegmentations(taxonomy, true);
        QString suffix;
        if (totalSegs > 0)
          suffix += QString(" (%1)").arg(totalSegs);

        return item->data(role).toString() + suffix;
      }
      else if (Qt::CheckStateRole == role)
        return m_taxonomyVisibility[taxonomy];
      else
        return item->data(role);
    }
    case EspINA::SEGMENTATION:
        return item->data(role);

    default:
      Q_ASSERT(false);
      break;
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool TaxonomyProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;

  if (index.isValid())
  {
    ModelItemPtr item = indexPtr(index);
    if (Qt::CheckStateRole == role)
    {
      if (EspINA::TAXONOMY == item->type())
      {
        TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
        m_taxonomyVisibility[taxonomy] = value.toBool()?Qt::Checked:Qt::Unchecked;

        int rows = rowCount(index);
        for (int r=0; r<rows; r++)
        {
          setData(index.child(r,0), value, role);
        }
        emit dataChanged(index, index);

        result = true;
      } else if (EspINA::SEGMENTATION == item->type())
      {
        result = m_model->setData(mapToSource(index), value, role);
      }

      QModelIndex parentIndex = parent(index);
      if (parentIndex.isValid())
      {
        ModelItemPtr parentItem = indexPtr(parentIndex);
        TaxonomyElementPtr parentTaxonomy = taxonomyElementPtr(parentItem);
        int parentRows = rowCount(parentIndex);
        Qt::CheckState checkState;
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
        m_taxonomyVisibility[parentTaxonomy] = checkState;
        emit dataChanged(parentIndex, parentIndex);
      }
    }
    else
      result = m_model->setData(mapToSource(index), value, role);
  }

  return result;
}

//------------------------------------------------------------------------
bool TaxonomyProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}


//------------------------------------------------------------------------
int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_rootTaxonomies.size();

  // Cast to base type
  ModelItemPtr parentItem = indexPtr(parent);
  int rows = 0;

  if (EspINA::TAXONOMY == parentItem->type())
  {
    TaxonomyElementPtr taxonomy = taxonomyElementPtr(parentItem);
    rows = numTaxonomies(taxonomy) + numSegmentations(taxonomy);
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return mapFromSource(m_model->index(row,column,m_model->taxonomyRoot()));

  ModelItemPtr parentItem = indexPtr(parent);
  Q_ASSERT(EspINA::TAXONOMY == parentItem->type());
  TaxonomyElementPtr parentTaxonomy = taxonomyElementPtr(parentItem);
  Q_ASSERT(parentTaxonomy);

  int subTaxonomies = numTaxonomies(parentTaxonomy);
  if (row < subTaxonomies)
  {
    return mapFromSource(m_model->index(row, column, m_model->taxonomyIndex(parentTaxonomy)));
  }

  int segRow = row - subTaxonomies;
  Q_ASSERT(segRow < numSegmentations(parentTaxonomy));
  ModelItemPtr internalPtr = m_taxonomySegmentations[parentTaxonomy][segRow];

  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItemPtr childItem = indexPtr(child);

  QModelIndex parent;
  switch (childItem->type())
  {
    case EspINA::TAXONOMY:
    {
      TaxonomyElementPtr childTaxonmy = taxonomyElementPtr(childItem);
      QModelIndex sourceParent = m_model->taxonomyIndex(childTaxonmy->parent());
      parent = mapFromSource(sourceParent);
      break;
    }
    case EspINA::SEGMENTATION:
    {
      foreach(TaxonomyElementPtr taxonomy, m_taxonomySegmentations.keys())
      {
        if (m_taxonomySegmentations[taxonomy].contains(childItem))
        {
          parent = mapFromSource(m_model->taxonomyIndex(taxonomy));
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
QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  if (sourceIndex == m_model->taxonomyRoot() ||
    sourceIndex == m_model->sampleRoot()   ||
    sourceIndex == m_model->channelRoot()  ||
    sourceIndex == m_model->filterRoot()   ||
    sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  QModelIndex proxyIndex;
  ModelItemPtr sourceItem = indexPtr(sourceIndex);
  switch(sourceItem->type())
  {
    case EspINA::TAXONOMY:
    {
      //Taxonomies are shown in the same order than in the original model
      proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
      break;
    }
    case EspINA::SEGMENTATION:
    {
      SegmentationPtr seg = segmentationPtr(sourceItem);
      Q_ASSERT(seg);
      TaxonomyElementPtr taxonomy = seg->taxonomy().get();
      if (taxonomy)
      {
        int row = m_taxonomySegmentations[taxonomy].indexOf(seg);
        if (row >= 0)
        {
          row += numTaxonomies(taxonomy);
          proxyIndex = createIndex(row, 0, sourceIndex.internalPointer());
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
QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  switch (proxyItem->type())
  {
    case EspINA::TAXONOMY:
    {
      TaxonomyElementPtr proxyTaxonomy = taxonomyElementPtr(proxyItem);
      Q_ASSERT(proxyTaxonomy);
      sourceIndex = m_model->taxonomyIndex(proxyTaxonomy);
      break;
    }
    case EspINA::SEGMENTATION:
    {
      SegmentationPtr proxySeg = segmentationPtr(proxyItem);
      Q_ASSERT(proxySeg);
      sourceIndex = m_model->segmentationIndex(proxySeg);
      break;
    }
    default:
      Q_ASSERT(false);
      break;
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
Qt::ItemFlags TaxonomyProxy::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = QAbstractProxyModel::flags(index) | Qt::ItemIsDropEnabled;

  if (index.isValid())
  {
    ModelItemPtr sourceItem = indexPtr(index);
    if (EspINA::TAXONOMY == sourceItem->type())
      f = f | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    else if (EspINA::SEGMENTATION == sourceItem->type())
      f = f | Qt::ItemIsDragEnabled;
  }

  return f;
}

typedef QMap<int, QVariant> DraggedItem;

//------------------------------------------------------------------------
bool TaxonomyProxy::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{

  TaxonomyElementPtr root = m_model->taxonomy()->root().get();

  ModelItemPtr parentItem = NULL;
  if (parent.isValid())
    parentItem = indexPtr(parent);
  else
    parentItem = root;

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

    switch (itemData[TypeRole].toInt())
    {
      case EspINA::SEGMENTATION:
        source |= SegmentationSource;
        break;
      case EspINA::TAXONOMY:
        source |= TaxonomySource;
        break;
      default:
        source = InvalidSource;
        break;
    }

    draggedItems << itemData;
  }

  // Change Taxonomy
  if (SegmentationSource == source && parentItem != root)
  {
    SegmentationList sources;
    foreach(DraggedItem draggedItem , draggedItems)
    {
      ModelItemPtr item = reinterpret_cast<ModelItemPtr>(draggedItem[RawPointerRole].value<quintptr>());
      sources << segmentationPtr(item);
    }

    TaxonomyElementPtr newTaxonomy;
    if (EspINA::TAXONOMY == parentItem->type())
    {
      newTaxonomy = taxonomyElementPtr(parentItem);
    }
    else if (EspINA::SEGMENTATION == parentItem->type())
    {
      SegmentationPtr seg = segmentationPtr(parentItem);
      newTaxonomy = seg->taxonomy().get();
    }
    Q_ASSERT(newTaxonomy);

    emit segmentationsDragged(sources, newTaxonomy);
  }

  // Change taxonomy parent
  else if (TaxonomySource == source && EspINA::TAXONOMY == parentItem->type())
  {
    TaxonomyElementList sources;
    foreach(DraggedItem draggedItem , draggedItems)
    {
      ModelItemPtr item = reinterpret_cast<ModelItemPtr>(draggedItem[RawPointerRole].value<quintptr>());
      sources << taxonomyElementPtr(item);
    }
    emit taxonomiesDragged(sources, taxonomyElementPtr(parentItem));
  }
  else if (InvalidSource == source)
    return false;

  return true;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numSegmentations(QModelIndex taxIndex, bool recursive) const
{
  ModelItemPtr item = indexPtr(taxIndex);
  if (EspINA::TAXONOMY != item->type())
    return 0;

  TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
  int total = numSegmentations(taxonomy);
  if (recursive)
  {
    int numTax = numTaxonomies(taxonomy);
    for (int subTax = 0; subTax < numTax; subTax++)
    {
      total += numSegmentations(index(subTax, 0, taxIndex), true);
    }
  }
  return total;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numTaxonomies(QModelIndex taxIndex) const
{
  ModelItemPtr item = indexPtr(taxIndex);
  if (EspINA::TAXONOMY != item->type())
    return 0;

  TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
  return numTaxonomies(taxonomy);
}

//------------------------------------------------------------------------
QModelIndexList TaxonomyProxy::segmentations(QModelIndex taxIndex, bool recursive) const
{
  QModelIndexList segs;

  int start = numTaxonomies(taxIndex);
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
SegmentationList TaxonomyProxy::segmentations(TaxonomyElementPtr taxonomy,
                                              bool recursive) const
{
  SegmentationList segs;

  if (recursive)
  {
    foreach(TaxonomyElementSPtr subElement, taxonomy->subElements())
    {
      segs << segmentations(subElement.get(), recursive);
    }
  }

  foreach (ModelItemPtr item, m_taxonomySegmentations[taxonomy])
  {
    segs << segmentationPtr(item);
  }

  return segs;
}

//------------------------------------------------------------------------
QModelIndexList TaxonomyProxy::sourceIndices(const QModelIndex& parent, int start, int end) const
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
QModelIndexList TaxonomyProxy::proxyIndices(const QModelIndex& parent, int start, int end) const
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
void TaxonomyProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot() ||
      sourceParent == m_model->channelRoot()  ||
      sourceParent == m_model->filterRoot())
    return;

  if (sourceParent == m_model->taxonomyRoot())
  {
    beginInsertRows(QModelIndex(), start, end);
    for (int row = start; row <= end; row++)
    {
      ModelItemPtr       insertedItem     = indexPtr(m_model->index(row, 0, sourceParent));
      TaxonomyElementPtr insertedElement = taxonomyElementPtr(insertedItem);

      m_rootTaxonomies << insertedElement;

      addTaxonomicalElement(insertedElement);
    }
    endInsertRows();
  } else if (sourceParent == m_model->segmentationRoot())
  {
    QMap<TaxonomyElementPtr , QList<ModelItemPtr > > relations;
    for (int row=start; row <= end; row++)
    {
      QModelIndex sourceIndex = m_model->index(row, 0, sourceParent);
      ModelItemPtr sourceItem = indexPtr(sourceIndex);
      SegmentationPtr seg = segmentationPtr(sourceItem);
      TaxonomyElementPtr taxonomy = seg->taxonomy().get();
      if (taxonomy)
        relations[taxonomy] << sourceItem;
    }
    foreach(TaxonomyElementPtr taxonomy, relations.keys())
    {
      int numTaxs = numTaxonomies(taxonomy);
      int numSegs = numSegmentations(taxonomy);
      int startRow = numTaxs + numSegs;
      int endRow = startRow + relations[taxonomy].size() - 1;
      QModelIndex proxyTaxonomy = mapFromSource(m_model->taxonomyIndex(taxonomy));
      beginInsertRows(proxyTaxonomy, startRow, endRow);
      m_taxonomySegmentations[taxonomy] << relations[taxonomy];
      endInsertRows();
    }
  } else
  {
    ModelItemPtr parentItem = indexPtr(sourceParent);
    if (EspINA::TAXONOMY == parentItem->type())
    {
      beginInsertRows(mapFromSource(sourceParent), start, end);
      for (int row = start; row <= end; row++)
      {
        ModelItemPtr sourceRow = indexPtr(m_model->index(row, 0, sourceParent));
        TaxonomyElementPtr taxonomy = taxonomyElementPtr(sourceRow);
        TaxonomyElementPtr parentNode = taxonomy->parent();
        if (parentNode != m_model->taxonomy()->root().get())
          m_numTaxonomies[parentNode] += 1;
        m_numTaxonomies[taxonomy]      = taxonomy->subElements().size();
        m_taxonomyVisibility[taxonomy] = Qt::Checked;
      }
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->sampleRoot() ||
    sourceParent == m_model->channelRoot()  ||
    sourceParent == m_model->filterRoot())
    return;

  if (sourceParent == m_model->segmentationRoot())
  {
    for (int row=start; row <= end; row++)
    {
      QModelIndex   sourceIndex = m_model->index(row, 0, sourceParent);
      QModelIndex   proxyIndex  = mapFromSource(sourceIndex);
      ModelItemPtr  item = indexPtr(sourceIndex);
      SegmentationPtr seg  = segmentationPtr(item);
      TaxonomyElementPtr taxonomy = seg->taxonomy().get();
      Q_ASSERT(taxonomy);
      int segRow = m_taxonomySegmentations[taxonomy].indexOf(item);
      if (segRow >= 0)
      {
        int row = numTaxonomies(taxonomy) + segRow;
        beginRemoveRows(proxyIndex.parent(), row, row);
        m_taxonomySegmentations[taxonomy].removeAt(segRow);
        endRemoveRows();
      }
    }
    return;
  }else // Handles taxonomyRoot and taxonomyNodes
  {
    beginRemoveRows(mapFromSource(sourceParent), start, end);
    for (int row=start; row <= end; row++)
    {
      ModelItemPtr item = indexPtr(m_model->index(row, 0, sourceParent));
      TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
      removeTaxonomy(taxonomy);
    }
    endRemoveRows();
    return;
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                             const QModelIndex &destinationParent, int destinationRow)
{
  Q_ASSERT(sourceStart == sourceEnd);

  QModelIndex proxySourceParent       = mapFromSource(sourceParent);
  QModelIndex proxyDestionationParent = mapFromSource(destinationParent);

  beginMoveRows(proxySourceParent, sourceStart, sourceEnd,
                proxyDestionationParent, destinationRow);

  ModelItemPtr movingItem = indexPtr(sourceParent.child(sourceStart, 0));
  TaxonomyElementPtr movingTaxonomy = taxonomyElementPtr(movingItem);

  if (proxySourceParent.isValid())
  {
    ModelItemPtr sourceItem = indexPtr(proxySourceParent);
    TaxonomyElementPtr sourceTaxonomy = taxonomyElementPtr(sourceItem);
    m_numTaxonomies[sourceTaxonomy] -=1;
  } else
  {
    m_rootTaxonomies.removeOne(movingTaxonomy);
  }

  if (proxyDestionationParent.isValid())
  {
    ModelItemPtr destinationItem = indexPtr(destinationParent);
    TaxonomyElementPtr destinationTaxonomy = taxonomyElementPtr(destinationItem);

    m_numTaxonomies[destinationTaxonomy] +=1;
  } else
  {
    m_rootTaxonomies << movingTaxonomy;
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
  endMoveRows();
}


//------------------------------------------------------------------------
bool TaxonomyProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
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
SegmentationPtr TaxonomyProxy::findSegmentation(QString tooltip)
{
  foreach (TaxonomyElementPtr tax, m_taxonomySegmentations.keys())
  {
    foreach(ModelItemPtr seg, m_taxonomySegmentations[tax])
      if (seg->data(Qt::ToolTipRole) == tooltip)
        return segmentationPtr(seg);
  }

  return SegmentationPtr();
}


//------------------------------------------------------------------------
void TaxonomyProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources;

  indices(sourceTopLeft, sourceBottomRight, sources);

  foreach(QModelIndex source, sources)
  {
    if (source.parent() == m_model->segmentationRoot())
    {
      bool indexChanged = true;
      ModelItemPtr sourceItem = indexPtr(source);
      SegmentationPtr    segmentation = segmentationPtr(sourceItem);
      TaxonomyElementPtr prevTaxonomy = NULL;
      foreach(TaxonomyElementPtr taxonomy, m_taxonomySegmentations.keys())
      {
        if (m_taxonomySegmentations[taxonomy].contains(sourceItem))
        {
          indexChanged = taxonomy != segmentation->taxonomy().get();
          prevTaxonomy = taxonomy;
          break;
        }
      }
      if (prevTaxonomy && indexChanged)
      {
        QModelIndex source = mapFromSource(m_model->taxonomyIndex(prevTaxonomy));
        QModelIndex destination = mapFromSource(m_model->taxonomyIndex(segmentation->taxonomy()));
        int currentRow = numTaxonomies(prevTaxonomy) + m_taxonomySegmentations[prevTaxonomy].indexOf(sourceItem);
        int newRow = rowCount(destination);
        beginMoveRows(source, currentRow, currentRow, destination, newRow);
        m_taxonomySegmentations[prevTaxonomy].removeOne(sourceItem);
        m_taxonomySegmentations[segmentation->taxonomy().get()] << sourceItem;
        endMoveRows();
      }
    }
    QModelIndex proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      emit dataChanged(proxyIndex, proxyIndex);
    }
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_rootTaxonomies.clear();
    m_numTaxonomies.clear();
    m_taxonomySegmentations.clear();
    m_taxonomyVisibility.clear();
  }
  endResetModel();
}


//------------------------------------------------------------------------
bool idOrdered(SegmentationPtr seg1, SegmentationPtr seg2)
{
  return seg1->number() < seg2->number();
}

//------------------------------------------------------------------------
void TaxonomyProxy::addTaxonomicalElement(TaxonomyElementPtr taxonomicalElement)
{
  m_numTaxonomies[taxonomicalElement]      = taxonomicalElement->subElements().size();
  m_taxonomyVisibility[taxonomicalElement] = Qt::Checked;

  foreach(TaxonomyElementSPtr subTaxonomcialElement, taxonomicalElement->subElements())
  {
    addTaxonomicalElement(subTaxonomcialElement.get());
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::removeTaxonomy(TaxonomyElementPtr taxonomy)
{
  // First remove its subtaxonomies
  foreach(TaxonomyElementSPtr subTax, taxonomy->subElements())
  {
    removeTaxonomy(subTax.get());
  }

  m_rootTaxonomies.removeOne(taxonomy); //Safe even if it's not root taxonomy
  m_numTaxonomies.remove(taxonomy);
  m_taxonomySegmentations.remove(taxonomy);
  TaxonomyElementPtr parentNode = taxonomy->parent();
  if (parentNode != m_model->taxonomy()->root().get())
    m_numTaxonomies[parentNode] -= 1;
}

//------------------------------------------------------------------------
int TaxonomyProxy::numSegmentations(TaxonomyElementPtr taxonomy, bool recursive) const
{
  int total = m_taxonomySegmentations[taxonomy].size();
  if (recursive)
    foreach(TaxonomyElementSPtr subtax, taxonomy->subElements())
    {
      total += numSegmentations(subtax.get(), recursive);
    }

  return total;
}
//------------------------------------------------------------------------
int TaxonomyProxy::numTaxonomies(TaxonomyElementPtr taxonomy) const
{
  // We can't rely on source's model to deal with row counting
  return m_numTaxonomies[taxonomy];
}
