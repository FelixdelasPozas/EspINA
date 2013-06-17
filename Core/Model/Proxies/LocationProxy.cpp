/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "LocationProxy.h"

// Espina
#include "Core/Model/EspinaModel.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/QtModelUtils.h>
#include <Core/Relations.h>

// Qt
#include <QPixmap>
#include <QSet>

using namespace EspINA;

typedef QSet<ModelItemPtr> SegSet;

//------------------------------------------------------------------------
LocationProxy::LocationProxy(QObject* parent)
: QAbstractProxyModel(parent)
, m_model(NULL)
{
}

//------------------------------------------------------------------------
LocationProxy::~LocationProxy()
{
}

//------------------------------------------------------------------------
void LocationProxy::setSourceModel(EspinaModel *sourceModel)
{
  m_model = sourceModel;

  connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
  connect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  connect(m_model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(sourceModelReset()));

  QAbstractProxyModel::setSourceModel(m_model);

  sourceRowsInserted(m_model->sampleRoot(),0,m_model->rowCount(m_model->sampleRoot())-1);
}

//------------------------------------------------------------------------
QVariant LocationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItemPtr item = indexPtr(proxyIndex);
  if (EspINA::SAMPLE == item->type())
  {
    if (Qt::DecorationRole == role)
      return QColor(Qt::blue);
    else
      return item->data(role);
  } else if (EspINA::SEGMENTATION == item->type())
  {
    return item->data(role);
  }
  Q_ASSERT(false);

  return QAbstractProxyModel::data(proxyIndex, role);
}

//------------------------------------------------------------------------
bool LocationProxy::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}


//------------------------------------------------------------------------
int LocationProxy::rowCount(const QModelIndex& parent) const
{
  int rows = 0;

  if (parent.isValid())
  {
    ModelItemPtr node = indexPtr(parent);
    rows = m_subNodes[node].size();
  } else 
  {
    rows = m_rootNodes.size();
  }

  return rows;
}

//------------------------------------------------------------------------
QModelIndex LocationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  ModelItemPtr internalPtr;
  if (parent.isValid())
  {
    ModelItemPtr node = indexPtr(parent);
    internalPtr = m_subNodes[node].at(row);
  } else
  {
    internalPtr = m_rootNodes.at(row);
  }

  return createIndex(row, column, internalPtr);
}

// WARNING: Don't use mapFromSource to implement model primitives!
//------------------------------------------------------------------------
QModelIndex LocationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItemPtr childNode = indexPtr(child);
  ModelItemPtr parent    = parentNode(childNode);
  if (!parent)
    return QModelIndex();

  int row = m_rootNodes.indexOf(parent);
  if (row < 0)
  {
    ModelItemPtr grandParent = parentNode(parent);
    row = m_subNodes[grandParent].indexOf(parent);
  }

  return createIndex(row, 0, parent);
}

//------------------------------------------------------------------------
QModelIndex LocationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  QModelIndex sourceParent = sourceIndex.parent();
  if (sourceParent != m_model->sampleRoot() &&
      sourceParent != m_model->segmentationRoot())
    return QModelIndex();

  ModelItemPtr sourceItem = indexPtr(sourceIndex);

  int row = -1;
  if (m_rootNodes.contains(sourceItem))
  {
    row = m_rootNodes.indexOf(sourceItem);
  } else
  {
    int k = 0;
    while (row < 0 && k < m_subNodes.size())
    {
      row = m_subNodes[m_subNodes.keys().at(k)].indexOf(sourceItem);
      k++;
    }
  }
  Q_ASSERT(row >= 0);
  return createIndex(row, 0, sourceItem);
}

//------------------------------------------------------------------------
QModelIndex LocationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);
  Q_ASSERT(proxyItem);

  return m_model->index(proxyItem);
}

//------------------------------------------------------------------------
void LocationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent != m_model->sampleRoot() &&
      sourceParent != m_model->segmentationRoot())
    return;

  // Inserted items don't have relationships
  // thus we just add them to the root list
  int proxyStart = m_rootNodes.size();
  int proxyEnd   = proxyStart + (end - start);
  beginInsertRows(QModelIndex(), proxyStart, proxyEnd);
  for (int row=start; row <= end; row++)
  {
    QModelIndex sourceIndex = sourceParent.child(row, 0);
    ModelItemPtr item = indexPtr(sourceIndex);

    registerNodes(item);
  }
  endInsertRows();
}

/// PRE: ModelItem has no relationships
//------------------------------------------------------------------------
void LocationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent != m_model->sampleRoot() &&
      sourceParent != m_model->segmentationRoot())
    return;

  for (int row=start; row <= end; row++)
  {
    QModelIndex     sourceIndex = sourceParent.child(row, 0);
    QModelIndex     proxyIndex  = mapFromSource(sourceIndex);
    ModelItemPtr    item        = indexPtr(sourceIndex);

    int proxyRow = proxyIndex.row();

    beginRemoveRows(proxyIndex.parent(), proxyRow, proxyRow);
    {
      m_rootNodes.removeOne(item);
      removeSubNodes(item);
    }
    endRemoveRows();
  }
}

//------------------------------------------------------------------------
void LocationProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
}

//------------------------------------------------------------------------
void LocationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  QModelIndexList sources = QtModelUtils::indices(sourceTopLeft, sourceBottomRight);

  foreach(QModelIndex source, sources)
  {
    QModelIndex proxyIndex = mapFromSource(source);
    if (proxyIndex.isValid())
    {
      ModelItemPtr proxyItem = indexPtr(proxyIndex);

      ModelItemPtr   prevLocation = parentNode(proxyItem);
      ModelItemSList relatedItems = proxyItem->relatedItems(EspINA::RELATION_IN, Relations::LOCATION);

      QModelIndex oldParent = proxyIndex.parent();
      int fromRow = proxyIndex.row();
      ModelItemPtr newLocation = NULL;
      int          toRow       = m_rootNodes.size();
      QModelIndex  newParent;

      if (!relatedItems.isEmpty())
      {
        newLocation = relatedItems.first().get();
        toRow       = m_subNodes[newLocation].size();
        newParent   = mapFromSource(m_model->index(newLocation));
      }

      if (prevLocation != newLocation)
      {
        beginMoveRows(oldParent, fromRow, fromRow, newParent, toRow);
        {
          if (prevLocation == NULL)
          {
            m_rootNodes.removeOne(proxyItem);
          } else 
          {
            m_subNodes[prevLocation].removeOne(proxyItem);
          }

          if (newLocation == NULL)
          {
            m_rootNodes << proxyItem;
          } else
          {
            m_subNodes[newLocation] << proxyItem;
          }
        }
        endMoveRows();
      } else
      {
        emit dataChanged(proxyIndex, proxyIndex);
      }
    }
  }
}

//------------------------------------------------------------------------
void LocationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_rootNodes.clear();
    m_subNodes.clear();
  }
  endResetModel();
}

//------------------------------------------------------------------------
ModelItemPtr LocationProxy::parentNode(const ModelItemPtr node) const
{
  ModelItemPtr parent = NULL;

  if (!m_rootNodes.contains(node))
  {
    int k = 0;
    while (!parent && k < m_subNodes.size())
    {
      ModelItemPtr key = m_subNodes.keys().at(k);
      if (m_subNodes[key].contains(node))
        parent = key;
      k++;
    }
  }

  return parent;
}

//------------------------------------------------------------------------
void LocationProxy::registerNodes(ModelItemPtr node)
{
  ModelItemSList parentItems = node->relatedItems(EspINA::RELATION_IN, Relations::LOCATION);
  if (parentItems.isEmpty())
  {
    if (!m_rootNodes.contains(node))
    {
      m_rootNodes << node;
    }
  } else {
    Q_ASSERT(parentItems.size() == 1);
    ModelItemPtr parentNode = parentItems.first().get();
    if (!m_subNodes[parentNode].contains(node))
    {
      m_subNodes[parentNode] << node;
    }
  }

  foreach(ModelItemSPtr subItem, node->relatedItems(EspINA::RELATION_OUT, Relations::LOCATION))
  {
    registerNodes(subItem.get());
  }
}

//------------------------------------------------------------------------
void LocationProxy::removeSubNodes(ModelItemPtr node)
{
  if (m_subNodes.contains(node))
  {
    foreach(ModelItemPtr subNode, m_subNodes[node])
    {
      removeSubNodes(subNode);
    }
    m_subNodes.remove(node);
  }
}
