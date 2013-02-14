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
#include <Core/Relations.h>
#include <QMimeData>

using namespace EspINA;

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
    ModelItemPtr parentItem = indexPtr(parent);
    Q_ASSERT(EspINA::SEGMENTATION == parentItem->type());
    SegmentationPtr seg = segmentationPtr(parentItem);
    rows = m_components[seg].size();
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex CompositionProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  ModelItemPtr internalPtr;
  if (!parent.isValid()) // Root segmentations
  {
    internalPtr = m_rootSegmentations.at(row);
  }
  else
  {
    ModelItemPtr parentItem = indexPtr(parent);
    Q_ASSERT(EspINA::SEGMENTATION == parentItem->type());
    SegmentationPtr seg = segmentationPtr(parentItem);
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

  ModelItemPtr childItem = indexPtr(child);
  Q_ASSERT(EspINA::SEGMENTATION == childItem->type());

  int i = 0;
  SegmentationPtr parentSeg = NULL;
  while(!parentSeg && i < m_components.size())
  {
    SegmentationPtr seg = m_components.keys().at(i);
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

  ModelItemPtr parentItem = parentSeg;
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

  ModelItemPtr sourceItem = indexPtr(sourceIndex);

  if (EspINA::SEGMENTATION != sourceItem->type())
    return QModelIndex();

  SegmentationPtr seg = segmentationPtr(sourceItem);

  ModelItemSList compositions = seg->relatedItems(EspINA::IN,
                                                  Relations::COMPOSITION);

  Q_ASSERT(compositions.size() <= 1);

  int row = -1;

  if (compositions.isEmpty())
    row = m_rootSegmentations.indexOf(seg);
  else
  {
    SegmentationPtr superSeg = segmentationPtr(compositions.first().data());
    row = m_components[superSeg].indexOf(sourceItem);
  }

  return createIndex(row, 0, sourceIndex.internalPointer());
}

//------------------------------------------------------------------------
QModelIndex CompositionProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  Q_ASSERT(EspINA::SEGMENTATION == proxyItem->type());
  SegmentationPtr proxySeg = segmentationPtr(proxyItem);
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
  SegmentationPtr newParentSeg;
  if (parent.isValid())
  {
    ModelItemPtr parentItem = indexPtr(parent);
    Q_ASSERT(EspINA::SEGMENTATION == parentItem->type());

    newParentSeg = segmentationPtr(parentItem);
    Q_ASSERT(newParentSeg);
  }

  // Recover dragged item information
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  SegmentationList draggedSegs;

  while (!stream.atEnd())
  {
    int row, col;
    QMap<int,  QVariant> roleDataMap;
    stream >> row >> col >> roleDataMap;

    QString segName = roleDataMap[Qt::ToolTipRole].toString();
    SegmentationPtr seg = findSegmentation(segName);
    Q_ASSERT(seg);
    SegmentationSPtr segPtr = m_sourceModel->findSegmentation(seg);

    SegmentationPtr prevParentSeg = parentSegmentation(seg);
    if (prevParentSeg)
    {
      SegmentationSPtr prevParentSegPtr = m_sourceModel->findSegmentation(prevParentSeg);
      m_sourceModel->removeRelation(prevParentSegPtr, segPtr, Relations::COMPOSITION);
    }
    if (newParentSeg)
    {
      SegmentationSPtr newParentSegPtr = m_sourceModel->findSegmentation(newParentSeg);
      m_sourceModel->addRelation(newParentSegPtr, segPtr, Relations::COMPOSITION);
    }
  }

  return true;
}

//------------------------------------------------------------------------
void CompositionProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  // Ignore all non-segmentation items
  if ( sourceParent != m_sourceModel->segmentationRoot())
    return;

  // Inserted segmentations don't have relationships
  // thus we just add them to the root list
  int proxyStart = m_rootSegmentations.size();
  int proxyEnd   = proxyStart + (end - start);
  beginInsertRows(QModelIndex(), proxyStart, proxyEnd);
  for (int row=start; row <= end; row++)
  {
    QModelIndex sourceIndex = sourceParent.child(row, 0);
    ModelItemPtr sourceItem = indexPtr(sourceIndex);
    Q_ASSERT(EspINA::SEGMENTATION == sourceItem->type());

    m_rootSegmentations << segmentationPtr(sourceItem);
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
  if ( sourceParent != m_sourceModel->segmentationRoot())
    return;

  for (int row=start; row <= end; row++)
  {
    QModelIndex     sourceIndex = sourceParent.child(row, 0);
    QModelIndex     proxyIndex  = mapFromSource(sourceIndex);
    ModelItemPtr    item        = indexPtr(sourceIndex);
    SegmentationPtr seg         = segmentationPtr(item);

    int proxyRow = proxyIndex.row();

    beginRemoveRows(proxyIndex.parent(), proxyRow, proxyRow);

    m_rootSegmentations.removeAll(seg);
    foreach(SegmentationPtr key, m_components.keys())
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
      ModelItemPtr    sourceItem  = indexPtr(sourceParent.child(row, 0));
      SegmentationPtr seg         = segmentationPtr(sourceItem);
      SegmentationPtr parentSeg   = parentSegmentation(sourceItem);

      int i = 0;
      SegmentationPtr prevParentSeg;
      while (!prevParentSeg && i < m_components.size())
      {
        SegmentationPtr key = m_components.keys().at(i);
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
SegmentationPtr CompositionProxy::findSegmentation(QString tooltip) const
{
  foreach (SegmentationPtr seg, m_components.keys())
    if (seg->data(Qt::ToolTipRole) == tooltip)
      return seg;

  return SegmentationPtr();
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
SegmentationPtr CompositionProxy::parentSegmentation(ModelItemPtr segItem) const
{
  ModelItemSList parentItem = segItem->relatedItems(EspINA::IN,
                                                    Relations::COMPOSITION);

  SegmentationPtr parentSeg = NULL;
  if (!parentItem.isEmpty())
  {
    Q_ASSERT(parentItem.size() == 1);
    parentSeg = segmentationPtr(parentItem.first().data());
  }

  return parentSeg;
}
