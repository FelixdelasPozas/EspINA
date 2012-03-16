/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "InformationProxy.h"

#include "common/model/EspinaModel.h"

//------------------------------------------------------------------------
void InformationProxy::setSourceModel(EspinaModel* sourceModel)
{
  m_model = sourceModel;
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  QAbstractProxyModel::setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  if (sourceIndex == m_model->taxonomyRoot() ||
    sourceIndex == m_model->sampleRoot()   ||
    sourceIndex == m_model->channelRoot()  ||
    sourceIndex == m_model->filterRoot()   ||
    sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  ModelItem *sourceItem = indexPtr(sourceIndex);
  Q_ASSERT(sourceItem);
  if (ModelItem::SEGMENTATION == sourceItem->type())
    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());

  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItem *proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  QModelIndex sourceIndex;
  if (ModelItem::SEGMENTATION == proxyItem->type())
  {
    Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
    Q_ASSERT(proxySeg);
    sourceIndex = m_model->segmentationIndex(proxySeg);
  }

  return sourceIndex;
}

//------------------------------------------------------------------------
int InformationProxy::columnCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_query.size();
return m_query.size();
  return 0;
}

//------------------------------------------------------------------------
int InformationProxy::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
    return m_model->rowCount(m_model->segmentationRoot());
  else
    return 0;// There are no sub-segmentations
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItem *childItem = indexPtr(child);
  Q_ASSERT(childItem);
  Q_ASSERT(ModelItem::SEGMENTATION == childItem->type());
  return mapFromSource(m_model->segmentationRoot());
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    // We need to forward all column indices to the same original index while keeping
    // column correct
    QModelIndex sourceIndex = m_model->index(row, 0, m_model->segmentationRoot());
    return createIndex(row, column, sourceIndex.internalPointer());
  }

  return QModelIndex();
}

//------------------------------------------------------------------------
QVariant InformationProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::DisplayRole == role)
  {
    return m_query[section];
  }

  return QAbstractProxyModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant InformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  ModelItem *proxyItem = indexPtr(proxyIndex);
  if (ModelItem::SEGMENTATION != proxyItem->type())
    return QVariant();

  if (role == Qt::DisplayRole)
  {
    QString query = m_query[proxyIndex.column()];
    if ("Name" == query)
      return proxyItem->data(Qt::DisplayRole);
    else
      return proxyItem->information(query);
  } else if (proxyIndex.column() > 0)
    return QVariant();//To avoid checkrole or other roles

  return QAbstractProxyModel::data(proxyIndex, role);
}


//------------------------------------------------------------------------
void InformationProxy::setQuery(const QStringList query)
{
  m_query = query;
}


//------------------------------------------------------------------------
void InformationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (sourceParent == m_model->segmentationRoot())
  {
    beginInsertRows(mapFromSource(sourceParent), start, end);
    endInsertRows();
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (sourceParent == m_model->segmentationRoot())
  {
    beginRemoveRows(mapFromSource(sourceParent), start, end);
    endRemoveRows();
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  if (sourceTopLeft.parent() == m_model->segmentationRoot())
  {
    emit dataChanged(mapFromSource(sourceTopLeft), mapFromSource(sourceBottomRight));
  }
}
