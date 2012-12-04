/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "CompositionProxy.h"

#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <QMimeData>

//------------------------------------------------------------------------
CompositionProxy::CompositionProxy(QObject* parent)
: QAbstractProxyModel(parent)
, m_sourceModel(NULL)
{
}

//------------------------------------------------------------------------
CompositionProxy::~CompositionProxy()
{
}

//------------------------------------------------------------------------
void CompositionProxy::setSourceModel(EspinaModel *sourceModel)
{
  m_sourceModel = sourceModel;

  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

  QAbstractProxyModel::setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
QVariant CompositionProxy::data(const QModelIndex& proxyIndex, int role) const
{
  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool CompositionProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}

// We need to rely on our own row count for each item in the proxy's model
// If we rely on the source's model, there are some inconsistencies during
// rows insertion/deletion
//------------------------------------------------------------------------
int CompositionProxy::rowCount(const QModelIndex& parent) const
{
  int rows = 0;

  if (!parent.isValid())
  {
    rows =  m_rootSegmentations.size();
  } else
  {
    ModelItem *parentItem = indexPtr(parent);
    Q_ASSERT(ModelItem::SEGMENTATION == parentItem->type());
    Segmentation *seg = dynamic_cast<Segmentation *>(parentItem);
    rows = m_components[seg].size();
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex CompositionProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  ModelItem *internalPtr;
  if (!parent.isValid()) // Root segmentations
  {
    internalPtr = m_rootSegmentations.at(row);
  }
  else
  {
    ModelItem *parentItem = indexPtr(parent);
    Q_ASSERT(ModelItem::SEGMENTATION == parentItem->type());
    Segmentation *seg = dynamic_cast<Segmentation *>(parentItem);
    Q_ASSERT(seg);
    internalPtr = m_components[seg][row];
  }

  return createIndex(row, column, internalPtr);
}

// WARNING: Don't use mapFromSource to implement model primitives!
//------------------------------------------------------------------------
QModelIndex CompositionProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItem *childItem = indexPtr(child);
  Q_ASSERT(childItem);
  Q_ASSERT(ModelItem::SEGMENTATION == childItem->type());

  int i = 0;
  Segmentation *parentSeg  = NULL;
  while(!parentSeg && i < m_components.size())
  {
    Segmentation *seg = m_components.keys().at(i);
    if (m_components[seg].contains(childItem))
      parentSeg = seg;
    i++;
  }

  if (!parentSeg)
    return QModelIndex();

  int row = m_rootSegmentations.indexOf(parentSeg);
  if (row < 0)
  {
    row = m_components[parentSeg].indexOf(childItem);
  }

  ModelItem *parentItem = parentSeg;
  return createIndex(row, 0, parentItem);
}

//------------------------------------------------------------------------
QModelIndex CompositionProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  // Ignore all non-segmentation items
  if ( sourceIndex == m_sourceModel->taxonomyRoot()
    || sourceIndex == m_sourceModel->sampleRoot()
    || sourceIndex == m_sourceModel->channelRoot()
    || sourceIndex == m_sourceModel->filterRoot()
    || sourceIndex == m_sourceModel->segmentationRoot() )
    return QModelIndex();

  ModelItem *sourceItem = indexPtr(sourceIndex);

  if (ModelItem::SEGMENTATION != sourceItem->type())
    return QModelIndex();

  Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
  Q_ASSERT(seg);

  ModelItem::Vector compositions = seg->relatedItems(ModelItem::IN,
                                                     Segmentation::COMPOSED_LINK);

  Q_ASSERT(compositions.size() <= 1);


  int row = -1;

  if (compositions.isEmpty())
    row = m_rootSegmentations.indexOf(seg);
  else
  {
    Segmentation *superSeg = dynamic_cast<Segmentation *>(compositions.first());
    row = m_components[superSeg].indexOf(sourceItem);
  }

  return createIndex(row, 0, sourceIndex.internalPointer());
}

//------------------------------------------------------------------------
QModelIndex CompositionProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItem *proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  Q_ASSERT(ModelItem::SEGMENTATION == proxyItem->type());
  Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
  Q_ASSERT(proxySeg);

  return m_sourceModel->segmentationIndex(proxySeg);
}

//------------------------------------------------------------------------
Qt::ItemFlags CompositionProxy::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = QAbstractProxyModel::flags(index);
  if (index.isValid())
    f = f | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
  else
    f = f | Qt::ItemIsDropEnabled;

  return f;
}

//------------------------------------------------------------------------
bool CompositionProxy::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  Segmentation *newParentSeg = NULL;
  if (parent.isValid())
  {
    ModelItem *parentItem = indexPtr(parent);
    Q_ASSERT(ModelItem::SEGMENTATION == parentItem->type());

    newParentSeg = dynamic_cast<Segmentation *>(parentItem);
    Q_ASSERT(newParentSeg);
  }

  // Recover dragged item information
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  QList<Segmentation *> draggedSegs;

  while (!stream.atEnd())
  {
    int row, col;
    QMap<int,  QVariant> roleDataMap;
    stream >> row >> col >> roleDataMap;

    QString segName = roleDataMap[Qt::ToolTipRole].toString();
    Segmentation *seg = findSegmentation(segName);
    Q_ASSERT(seg);

    Segmentation *prevParentSeg = parentSegmentation(seg);
    if (prevParentSeg)
      m_sourceModel->removeRelation(prevParentSeg, seg, Segmentation::COMPOSED_LINK);
    if (newParentSeg)
      m_sourceModel->addRelation(newParentSeg, seg, Segmentation::COMPOSED_LINK);
  }

  return true;
}

//------------------------------------------------------------------------
void CompositionProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  // Ignore all non-segmentation items
  if ( sourceParent == m_sourceModel->taxonomyRoot()
    || sourceParent == m_sourceModel->sampleRoot()
    || sourceParent == m_sourceModel->channelRoot()
    || sourceParent == m_sourceModel->filterRoot() )
    return;

  Q_ASSERT(sourceParent == m_sourceModel->segmentationRoot());

  // Inserted segmentations don't have relationships
  // thus we just add them to the root list
  int proxyStart = m_rootSegmentations.size();
  int proxyEnd   = proxyStart + (end - start);
  beginInsertRows(QModelIndex(), proxyStart, proxyEnd);
  for (int row=start; row <= end; row++)
  {
    QModelIndex sourceIndex = sourceParent.child(row, 0);
    ModelItem  *sourceItem  = indexPtr(sourceIndex);
    Q_ASSERT(ModelItem::SEGMENTATION == sourceItem->type());

    m_rootSegmentations << dynamic_cast<Segmentation *>(sourceItem);
  }
  endInsertRows();
}

/// PRE: Segmentation has no relationships
//------------------------------------------------------------------------
void CompositionProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  // Ignore all non-segmentation items
  if ( sourceParent == m_sourceModel->taxonomyRoot()
    || sourceParent == m_sourceModel->sampleRoot()
    || sourceParent == m_sourceModel->channelRoot()
    || sourceParent == m_sourceModel->filterRoot() )
    return;

  Q_ASSERT(sourceParent == m_sourceModel->segmentationRoot());

  for (int row=start; row <= end; row++)
  {
    QModelIndex   sourceIndex = sourceParent.child(row, 0);
    QModelIndex   proxyIndex  = mapFromSource(sourceIndex);
    ModelItem    *item        = indexPtr(sourceIndex);
    Segmentation *seg         = dynamic_cast<Segmentation *>(item);

    int proxyRow = proxyIndex.row();

    beginRemoveRows(proxyIndex.parent(), proxyRow, proxyRow);

    m_rootSegmentations.removeAll(seg);
    foreach(Segmentation *key, m_components.keys())
    {
      if (key == seg)
        m_components.remove(key);
      else
        m_components[key].removeAll(seg);
    }

    endRemoveRows();
  }
}

//------------------------------------------------------------------------
void CompositionProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
void CompositionProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  QModelIndex sourceParent = sourceTopLeft.parent();

  if (sourceParent != m_sourceModel->segmentationRoot())
    return;

  Q_ASSERT(sourceBottomRight.parent() == sourceParent);

  for(int row=sourceTopLeft.row(); row <= sourceBottomRight.row(); row++)
  {
      bool indexChanged = false;

      ModelItem    *sourceItem  = indexPtr(sourceParent.child(row, 0));
      Segmentation *seg         = dynamic_cast<Segmentation *>(sourceItem);
      Segmentation *parentSeg   = parentSegmentation(sourceItem);

      int i = 0;
      Segmentation *prevParentSeg = NULL;
      while (prevParentSeg == NULL && i < m_components.size())
      {
        Segmentation *key = m_components.keys().at(i);
        if (m_components[key].contains(sourceItem))
          prevParentSeg = key;
        i++;
      }

      QModelIndex proxyIndex;
      if (!prevParentSeg && parentSeg)
      {
        //qDebug() << "Add composition relationship
        QModelIndex destination = mapFromSource(m_sourceModel->segmentationIndex(parentSeg));
        int currentRow = m_rootSegmentations.indexOf(seg);
        int newRow     = m_components[parentSeg].size();
        beginMoveRows(QModelIndex(), currentRow, currentRow, destination, newRow);
        m_rootSegmentations.removeAll(seg);
        m_components[parentSeg] << sourceItem;
        endMoveRows();
        proxyIndex = destination.child(newRow, 0);
      } else if (prevParentSeg && !parentSeg)
      {
        //qDebug() << "Remove composition relationship
        QModelIndex source = mapFromSource(m_sourceModel->segmentationIndex(prevParentSeg));
        int currentRow = m_components[prevParentSeg].indexOf(seg);
        int newRow     = m_rootSegmentations.size();
        beginMoveRows(source, currentRow, currentRow, QModelIndex(), newRow);
        m_components[prevParentSeg].removeAll(sourceItem);
        m_rootSegmentations << seg;
        endMoveRows();
        proxyIndex = index(newRow, 0);
      } else if (prevParentSeg && parentSeg && prevParentSeg != parentSeg)
      {
        //qDebug() << "Modify composition relationship
        QModelIndex source      = mapFromSource(m_sourceModel->segmentationIndex(prevParentSeg));
        QModelIndex destination = mapFromSource(m_sourceModel->segmentationIndex(parentSeg));
        int currentRow = m_components[prevParentSeg].indexOf(sourceItem);
        int newRow = rowCount(destination);
        beginMoveRows(source, currentRow, currentRow, destination, newRow);
        m_components[prevParentSeg].removeAll(sourceItem);
        m_components[parentSeg] << sourceItem;
        endMoveRows();
        proxyIndex = destination.child(newRow, 0);
      }

      if (proxyIndex.isValid())
        emit dataChanged(proxyIndex, proxyIndex);
  }
}

//------------------------------------------------------------------------
Segmentation* CompositionProxy::findSegmentation(QString tooltip) const
{
  foreach (Segmentation *seg, m_components.keys())
    if (seg->data(Qt::ToolTipRole) == tooltip)
      return seg;

  return NULL;
}

//------------------------------------------------------------------------
bool CompositionProxy::indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result)
{
  result << topLeft;

  if (topLeft == bottomRight)
    return true;

  for (int r = 0; r < m_sourceModel->rowCount(topLeft); r++)
  {
    if (indices(topLeft.child(r, 0), bottomRight, result))
      return true;
  }

  for (int r = topLeft.row(); r < m_sourceModel->rowCount(topLeft.parent()); r++)
    if (indices(topLeft.sibling(r,0), bottomRight, result))
      return true;

  return false;
}

//------------------------------------------------------------------------
Segmentation *CompositionProxy::parentSegmentation(ModelItem* segItem) const
{
  ModelItem::Vector parentItem = segItem->relatedItems(ModelItem::IN,
                                                       Segmentation::COMPOSED_LINK);

  Segmentation *parentSeg = NULL;
  if (!parentItem.isEmpty())
  {
    Q_ASSERT(parentItem.size() == 1);
    parentSeg = dynamic_cast<Segmentation *>(parentItem.first());
  }

  return parentSeg;
}
