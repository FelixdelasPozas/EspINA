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
#include <Core/Model/QtModelUtils.h>
#include <Core/Extensions/SegmentationExtension.h>

using namespace EspINA;

//------------------------------------------------------------------------
TaxonomicalInformationProxy::TaxonomicalInformationProxy()
: QAbstractProxyModel()
, m_model(NULL)
{

}

//------------------------------------------------------------------------
void TaxonomicalInformationProxy::setSourceModel(EspinaModel *sourceModel)
{
  if (m_model)
  {
    disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
//     disconnect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
//                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
               this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
//     disconnect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//                this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
//     disconnect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//                this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    disconnect(m_model, SIGNAL(modelAboutToBeReset()),
               this, SLOT(sourceModelReset()));
  }

  m_information.clear();

  m_model = sourceModel;

  if (m_model)
  {
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
//     connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
//             this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
//     connect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
//     connect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//             this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_model, SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelReset()));
  }

  QAbstractProxyModel::setSourceModel(sourceModel);

  sourceRowsInserted(m_model->segmentationRoot(), 0, m_model->rowCount(m_model->segmentationRoot()) - 1);
}

//------------------------------------------------------------------------
QVariant TaxonomicalInformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

  ModelItemPtr item = indexPtr(proxyIndex);
  switch (item->type())
  {
    case EspINA::TAXONOMY:
    {
      TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
      if (InformationTagsRole == role)
      {
        return m_information[taxonomy].Tags;
      } else if (Qt::DisplayRole == role)
      {
        return taxonomy->qualifiedName();
      }

    } break;

    case EspINA::SEGMENTATION:
    {
      if (role == Qt::TextAlignmentRole)
        return Qt::AlignRight;

      SegmentationPtr    segmentation = segmentationPtr(item);
      TaxonomyElementPtr segTaxonomy  = segmentation->taxonomy().data();
      const QStringList &tags = m_information[segTaxonomy].Tags;
      if (role == Qt::DisplayRole && !tags.isEmpty())
      {
        return segmentation->information(tags[proxyIndex.column()]);
      }

    } break;

    default:
      return QVariant();
  };

  // Avoid other roles for non initial column (checkrole, decoration, etc)
  if (proxyIndex.column() > 0)
    return QVariant();
  else
    return QAbstractProxyModel::data(proxyIndex, role);
}


//------------------------------------------------------------------------
bool TaxonomicalInformationProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool ok = false;
  if (index.isValid())
  {
    ModelItemPtr item = indexPtr(index);
    if (EspINA::TAXONOMY == item->type() && InformationTagsRole == role)
    {
      if (m_information[item].Tags != value.toStringList())
      {
        QModelIndex parent = index.parent();
        beginRemoveColumns(index, 0, columnCount(index));
        m_information[item].Tags.clear();
        endRemoveColumns();
        QStringList tags = value.toStringList();
        beginInsertColumns(index, 0, tags.size() - 1);
        m_information[item].Tags = tags;
        endInsertColumns();
        ok = true;
      }
    }
  }

  return ok;
}

//------------------------------------------------------------------------
QModelIndex TaxonomicalInformationProxy::mapFromSource(const QModelIndex& sourceIndex) const
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
  if (EspINA::TAXONOMY == sourceItem->type())
  {
    int row = m_taxonomies.indexOf(sourceItem);
    proxyIndex = createIndex(row, sourceIndex.column(), sourceItem);
  }
  else if (EspINA::SEGMENTATION == sourceItem->type())
  {
    SegmentationPtr    segmentation = SegmentationPtr(sourceItem);
    TaxonomyElementPtr taxonomy     = segmentation->taxonomy().data();
    int row = m_information[taxonomy].Nodes.indexOf(segmentation);
    proxyIndex = createIndex(row, sourceIndex.column(), sourceItem);
  }

  return proxyIndex;
}

//------------------------------------------------------------------------
QModelIndex TaxonomicalInformationProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  ModelItemPtr proxyItem = indexPtr(proxyIndex);

  QModelIndex sourceIndex;
  if (EspINA::SEGMENTATION == proxyItem->type() || EspINA::TAXONOMY == proxyItem->type())
    sourceIndex = m_model->index(proxyItem);

  return sourceIndex;
}

//------------------------------------------------------------------------
bool TaxonomicalInformationProxy::hasChildren(const QModelIndex &parent) const
{
  return rowCount(parent) > 0 && columnCount(parent) > 0;
}

//------------------------------------------------------------------------
int TaxonomicalInformationProxy::rowCount(const QModelIndex& parent) const
{
  int rows = 0;

  if (m_taxonomies.isEmpty())
    rows = 0;
  else if (!parent.isValid())
    rows = m_taxonomies.size();
  else {
    ModelItemPtr taxonomy = indexPtr(parent);
    rows = m_information[taxonomy].Nodes.size();
  }

  return rows;
}

//------------------------------------------------------------------------
int TaxonomicalInformationProxy::columnCount(const QModelIndex& parent) const
{
  int count = 1;

  if (parent.isValid())
  {
    ModelItemPtr parentItem = indexPtr(parent);
    count = m_information[parentItem].Tags.size();
    count = count?count:1;
  }

  return count;
}

//------------------------------------------------------------------------
QModelIndex TaxonomicalInformationProxy::parent(const QModelIndex& child) const
{
  QModelIndex parentIndex;

  if (child.isValid())
  {
    ModelItemPtr childItem = indexPtr(child);
    if (EspINA::SEGMENTATION == childItem->type())
    {
      SegmentationPtr segmentation = segmentationPtr(childItem);
      ModelItemPtr    taxonomy     = segmentation->taxonomy().data();

      parentIndex = createIndex(m_taxonomies.indexOf(taxonomy), 0, taxonomy);
    }
  }

  return parentIndex;
}

//------------------------------------------------------------------------
QModelIndex TaxonomicalInformationProxy::index(int row, int column, const QModelIndex& parent) const
{
  QModelIndex child;

  if (hasIndex(row, column, parent))
  {
    if (!parent.isValid())
    {
      child = createIndex(row, column, m_taxonomies[row]);
    } else 
    {
      ModelItemPtr taxonomy = indexPtr(parent);
      child = createIndex(row, column, m_information[taxonomy].Nodes[row]);
    }
  }

  return child;
}

//------------------------------------------------------------------------
void TaxonomicalInformationProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  if (!sourceParent.isValid())
    return;

  if (sourceParent == m_model->segmentationRoot())
  {
    for (int row = start; row <= end; ++row)
    {
      ModelItemPtr    child        = indexPtr(sourceParent.child(row, 0));
      SegmentationPtr segmentation = segmentationPtr(child);
      ModelItemPtr    taxonomy     = segmentation->taxonomy().data();

      int parentRow = m_taxonomies.indexOf(taxonomy);

      if (parentRow < 0)
      {
        parentRow = m_taxonomies.size();
        beginInsertRows(QModelIndex(), parentRow, parentRow);
        m_taxonomies << taxonomy;
        endInsertRows();
      }

      QModelIndex parent = createIndex(parentRow, 0, taxonomy);

      int nextRow  = m_information[taxonomy].Nodes.size();
      beginInsertRows(parent, nextRow, nextRow);
      m_information[taxonomy].Nodes << segmentation;
      endInsertRows();
    }
  }
}

//------------------------------------------------------------------------
void TaxonomicalInformationProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  if (sourceParent == m_model->segmentationRoot())
  {
    for (int row = start; row <= end; ++row)
    {
      ModelItemPtr    child        = indexPtr(sourceParent.child(row, 0));
      SegmentationPtr segmentation = segmentationPtr(child);
      ModelItemPtr    taxonomy     = segmentation->taxonomy().data();

      int taxonomyRow     = m_taxonomies.indexOf(taxonomy);
      QModelIndex parent  = createIndex(taxonomyRow, 0, taxonomy);
      int segmentationRow = m_information[taxonomy].Nodes.indexOf(segmentation);

      beginRemoveRows(parent, segmentationRow, segmentationRow);
      m_information[taxonomy].Nodes.removeAt(segmentationRow);
      endRemoveRows();

      if (m_information[taxonomy].Nodes.isEmpty())
      {
        beginRemoveRows(QModelIndex(), taxonomyRow, taxonomyRow);
        m_information.remove(taxonomy);
        Q_ASSERT(m_taxonomies.removeOne(taxonomy));
        endRemoveRows();
      }
    }
  }
}

//------------------------------------------------------------------------
void TaxonomicalInformationProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  foreach(QModelIndex source, QtModelUtils::indices(sourceTopLeft, sourceBottomRight))
  {
    if (source.parent() == m_model->segmentationRoot())
    {
      bool indexChanged = true;
      SegmentationPtr    segmentation = segmentationPtr(indexPtr(source));
      TaxonomyElementPtr prevTaxonomy = NULL;

      foreach(ModelItemPtr key, m_information.keys())
      {
        TaxonomyElementPtr taxonomy = taxonomyElementPtr(key);
        if (m_information[taxonomy].Nodes.contains(segmentation))
        {
          indexChanged = taxonomy != segmentation->taxonomy();
          prevTaxonomy = taxonomy;
          break;
        }
      }

      if (prevTaxonomy && indexChanged)
      {
        // Remove segmentation from previous location
        int prevTaxonomyRow = m_taxonomies.indexOf(prevTaxonomy);
        QModelIndex prevParent  = createIndex(prevTaxonomyRow, 0, prevTaxonomy);
        int segmentationRow = m_information[prevTaxonomy].Nodes.indexOf(segmentation);

        beginRemoveRows(prevParent, segmentationRow, segmentationRow);
        m_information[prevTaxonomy].Nodes.removeAt(segmentationRow);
        endRemoveRows();

        if (m_information[prevTaxonomy].Nodes.isEmpty())
        {
          beginRemoveRows(QModelIndex(), prevTaxonomyRow, prevTaxonomyRow);
          m_information.remove(prevTaxonomy);
          Q_ASSERT(m_taxonomies.removeOne(prevTaxonomy));
          endRemoveRows();
        }

        TaxonomyElementPtr taxonomy = segmentation->taxonomy().data();

        // Insert segmentation into new location
        int parentRow = m_taxonomies.indexOf(taxonomy);

        if (parentRow < 0)
        {
          parentRow = m_taxonomies.size();
          beginInsertRows(QModelIndex(), parentRow, parentRow);
          m_taxonomies << taxonomy;
          endInsertRows();
        }

        QModelIndex parent = createIndex(parentRow, 0, taxonomy);

        int nextRow  = m_information[taxonomy].Nodes.size();
        beginInsertRows(parent, nextRow, nextRow);
        m_information[taxonomy].Nodes << segmentation;
        endInsertRows();
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
void TaxonomicalInformationProxy::sourceModelReset()
{
  beginResetModel();
  {
    m_information.clear();
    m_taxonomies.clear();
  }
  endResetModel();
}
