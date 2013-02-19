/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
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


#include "InformationProxy.h"

#include "Core/Model/EspinaFactory.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Extensions/SegmentationExtension.h>

using namespace EspINA;

//------------------------------------------------------------------------
InformationProxy::InformationProxy()
: QAbstractProxyModel()
, m_model(NULL)
{

}

//------------------------------------------------------------------------
void InformationProxy::setSourceModel(EspinaModel *sourceModel)
{
  if (m_model)
  {
    disconnect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
    disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  }

  m_model = sourceModel;
  m_elements.clear();

  if (m_model)
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
  connect(m_model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(sourceModelReset()));

  sourceRowsInserted(m_model->segmentationRoot(), 0, m_model->segmentations().size()-1);

  QAbstractProxyModel::setSourceModel(sourceModel);
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();

  if ( sourceIndex == m_model->taxonomyRoot()
    || sourceIndex == m_model->sampleRoot()
    || sourceIndex == m_model->channelRoot()
    || sourceIndex == m_model->filterRoot()
    || sourceIndex == m_model->segmentationRoot())
    return QModelIndex();

  ModelItemPtr sourceItem = indexPtr(sourceIndex);

  QModelIndex proxyIndex;
  if (EspINA::SEGMENTATION == sourceItem->type())
    proxyIndex = createIndex(sourceIndex.row(), sourceIndex.column(), sourceItem);

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);

  QModelIndex sourceIndex;
  if (EspINA::SEGMENTATION == proxyItem->type())
    sourceIndex = m_model->index(proxyItem);

  return sourceIndex;
}

//------------------------------------------------------------------------
int InformationProxy::columnCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    return m_query.size();
  }

  return 0;
}

//------------------------------------------------------------------------
int InformationProxy::rowCount(const QModelIndex& parent) const
{
  if (m_query.isEmpty())
    return 0;

  if (!parent.isValid())
    return m_elements.size();
  else
    return 0;// There are no sub-segmentations
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();

  ModelItemPtr childItem = indexPtr(child);
  Q_ASSERT(EspINA::SEGMENTATION == childItem->type());
  return mapFromSource(m_model->segmentationRoot());
}

//------------------------------------------------------------------------
QModelIndex InformationProxy::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
  {
    // We need to forward all column indices to the same
    // original index while keeping column correct
    return createIndex(row, column, m_elements[row].internalPointer());
  }

  return QModelIndex();
}

//------------------------------------------------------------------------
QVariant InformationProxy::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (m_query.isEmpty())
    return QAbstractProxyModel::headerData(section, orientation, role);

  if (Qt::DisplayRole == role && section < m_query.size())
  {
    return m_query[section];
  }

  return QAbstractProxyModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant InformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);
  if (EspINA::SEGMENTATION != proxyItem->type())
    return QVariant();

  SegmentationPtr segmentation = segmentationPtr(proxyItem);

  if (role == Qt::TextAlignmentRole)
    return Qt::AlignRight;

  if (role == Qt::DisplayRole && !m_query.isEmpty())
  {
    Segmentation::InfoTag tag = m_query[proxyIndex.column()];
    if (!segmentation->availableInformations().contains(tag))
    {
      Segmentation::InformationExtension prototype = m_model->factory()->informationProvider(tag);
      segmentation->addExtension(prototype->clone());
    }
    return segmentation->information(tag);
  } else if (proxyIndex.column() > 0)
    return QVariant();//To avoid checkrole or other roles

    return QAbstractProxyModel::data(proxyIndex, role);
}


//------------------------------------------------------------------------
void InformationProxy::setQuery(const Segmentation::InfoTagList query)
{
  beginResetModel();
  m_query = query;
  endResetModel();
}

//------------------------------------------------------------------------
const Segmentation::InfoTagList InformationProxy::availableInformation() const
{
  if (m_elements.isEmpty())
    return QStringList();

  ModelItemPtr item = indexPtr(m_elements.first());
  return segmentationPtr(item)->availableInformations();
}



//------------------------------------------------------------------------
void InformationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
// Avoid population the view if no query is selected
{
  if (sourceParent == m_model->segmentationRoot())
  {
    if (m_query.isEmpty())
    {
//       QModelIndex sourceIndex = mapFromSource(m_model->index(start, 0, sourceParent));
//       if (sourceIndex.isValid())
//       {
//         ModelItemPtr item = indexPtr(sourceIndex);
//         setQuery(item->availableInformations());
//       }
    }
    // Avoid populating the view if no query is selected
    if (!m_query.isEmpty())
      beginInsertRows(mapFromSource(sourceParent), start, end);
    for (int row = start; row <= end; row++)
    {
      m_elements << mapFromSource(m_model->index(row, 0, sourceParent));
    }
    if (!m_query.isEmpty())
      endInsertRows();
  }
}

//------------------------------------------------------------------------
void InformationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (sourceParent == m_model->segmentationRoot())
  {
    // Avoid population the view if no query is selected
    if (!m_query.isEmpty())
      beginRemoveRows(mapFromSource(sourceParent), start, end);
    for (int row = start; row <= end; row++)
    {
      // We use start instead of row to avoid access to removed indices
      m_elements.removeAt(start);
    }
    if (!m_query.isEmpty())
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

//------------------------------------------------------------------------
void InformationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_elements.clear();
  }
  endResetModel();
}
