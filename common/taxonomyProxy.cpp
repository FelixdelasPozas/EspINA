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


#include "taxonomyProxy.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "espina.h"
#include "sample.h"
#include "segmentation.h"

#include <data/modelItem.h>
#include <data/taxonomy.h>
#include <qmimedata.h>


//------------------------------------------------------------------------
TaxonomyProxy::TaxonomyProxy(QObject* parent)
    : QAbstractProxyModel(parent)
{
  updateSegmentations();
}

//------------------------------------------------------------------------
TaxonomyProxy::~TaxonomyProxy()
{
}

//------------------------------------------------------------------------
void TaxonomyProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  QAbstractProxyModel::setSourceModel(sourceModel);
  updateSegmentations();
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
  connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this,SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
}

//------------------------------------------------------------------------
int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (!parent.isValid())
    return model->rowCount(model->taxonomyRoot());

  // Cast to base type
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    int numSegs = m_taxonomySegs[parentTax].size();
    return parentTax->getSubElements().size() + numSegs;
  }
  // Otherwise Samples and Segmentations have no children
  return 0;
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return mapFromSource(model->index(row,column,model->taxonomyRoot()));
  
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    IModelItem *element;
    int subTaxonomies = parentTax->getSubElements().size();
    if (row < subTaxonomies)
      element = parentTax->getSubElements()[row];
    else
      element = m_taxonomySegs[parentTax][row-subTaxonomies];

    return createIndex(row, column, element);
  }
  
  // Otherwise, invalid index: Neither Samples nor Segmentations can't be parent index
  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);

  if (!child.isValid())
    return QModelIndex();

  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert(childItem);
  // Checks if Taxonomy
  TaxonomyNode *childTax = dynamic_cast<TaxonomyNode *>(childItem);
  if (childTax)
  {
    QModelIndex sourceTax = model->taxonomyIndex(childTax);
    QModelIndex sourceParent = model->parent(sourceTax);
    return mapFromSource(sourceParent);
  }

  // Otherwise is a segmentation
  Segmentation *childSeg = dynamic_cast<Segmentation *>(childItem);
  if (childSeg)
    return mapFromSource(model->taxonomyIndex(childSeg->taxonomy()));

  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();
  
  if (sourceIndex == EspINA::instance()->taxonomyRoot())
    return QModelIndex();
  
  if (sourceIndex == EspINA::instance()->segmentationRoot())
    return QModelIndex();
  
  if (sourceIndex == EspINA::instance()->sampleRoot())
    return QModelIndex();
  
  IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
  TaxonomyNode * tax = dynamic_cast<TaxonomyNode *>(sourceItem);
  if (tax)
    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
  
  //There are no samples in the new model
  Sample *sample = dynamic_cast<Sample *>(sourceItem);
  if (sample)
    return QModelIndex();
  
  Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
  if (seg)
  {
    TaxonomyNode *parent = seg->taxonomy();
    int row = parent->getSubElements().size() + m_taxonomySegs[parent].indexOf(seg);
    return createIndex(row,0,sourceIndex.internalPointer());
  }
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();
  
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  IModelItem *proxyItem = static_cast<IModelItem *>(proxyIndex.internalPointer());
  
  TaxonomyNode *proxyTax = dynamic_cast<TaxonomyNode *>(proxyItem);
  if (proxyTax)
    return model->taxonomyIndex(proxyTax);
  
  Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
  if (proxySeg)
    return model->segmentationIndex(proxySeg);
  
  assert(false);//Samples are not allowed
  return QModelIndex();
}

//------------------------------------------------------------------------
Qt::ItemFlags TaxonomyProxy::flags(const QModelIndex& index) const
{
  IModelItem *sourceItem = static_cast<IModelItem *>(index.internalPointer());
  Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
  if (seg)
  {
    return QAbstractProxyModel::flags(index)  | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }else{
     return QAbstractProxyModel::flags(index) | Qt::ItemIsDropEnabled;
  }
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
bool TaxonomyProxy::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  TaxonomyNode *newTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (!newTax)
  {
    Segmentation *parentSeg = dynamic_cast<Segmentation *>(parentItem);
    if (!parentSeg)
      return true;//Unkown type
    newTax = parentSeg->taxonomy();
  }
  
  // Recover dragged item information
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);
  
  while (!stream.atEnd())
  {
    int row, col;
    QMap<int,  QVariant> roleDataMap;
    stream >> row >> col >> roleDataMap;
    
    Segmentation *draggedSeg = NULL;
    
    QString segName = roleDataMap[Qt::DisplayRole].toString();
    foreach (const TaxonomyNode *tax, m_taxonomySegs.keys())
    {
      foreach (Segmentation *seg, m_taxonomySegs[tax])
      {
	if (seg->data(Qt::DisplayRole) == segName)
	{
	  draggedSeg = seg;
	  break;
	}
      }
      if (draggedSeg)
	break;
    }
    
   if (draggedSeg)
   {
     EspINA *model = dynamic_cast<EspINA *>(sourceModel());
     QModelIndex oldTaxonomyIndex = mapFromSource(model->taxonomyIndex(draggedSeg->taxonomy()));
     int row = m_taxonomySegs[draggedSeg->taxonomy()].indexOf(draggedSeg);
     beginRemoveRows(oldTaxonomyIndex,row,row);
     model->changeTaxonomy(draggedSeg, newTax);
     endRemoveRows();
     beginInsertRows(parent,row,row);
     endInsertRows();
   }
  }

  return true;
}


//------------------------------------------------------------------------
QVariant TaxonomyProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (role != Qt::DisplayRole)
    return QAbstractProxyModel::data(proxyIndex, role);
  
  IModelItem *proxyItem = static_cast<IModelItem *>(proxyIndex.internalPointer());
  
  TaxonomyNode *proxyTax = dynamic_cast<TaxonomyNode *>(proxyItem);
  if (proxyTax)
  {
    return QString("%1 (%2)").arg(proxyTax->getName()).arg(m_taxonomySegs[proxyTax].size());
  }
  else
    return QAbstractProxyModel::data(proxyIndex, role);
}


//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
    return;

  if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    
    QModelIndex sourceIndex = model->index(start, 0, sourceParent);
    IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
//     qDebug() << sourceSeg->taxonomy()->getName() << " Inserted";
    assert(sourceSeg);
    TaxonomyNode *segParent = sourceSeg->taxonomy();
    QModelIndex parentIndex = mapFromSource(model->taxonomyIndex(segParent));
    int row = m_taxonomySegs[segParent].indexOf(sourceSeg);
    beginInsertRows(parentIndex, row, row);
    endInsertRows();
    emit dataChanged(parentIndex,parentIndex);
  } else // In case sourceParent is taxonomyRoot, proxyParent will be an invalid index
  {
      beginInsertRows(mapFromSource(sourceParent),start,end);
      endInsertRows();
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  if (sourceParent == model->sampleRoot())
    return;
  
  if (sourceParent == model->segmentationRoot())
  {
    assert(start == end);
    // Need to find its parent before deletion
    QModelIndex sourceIndex = model->index(start, 0, sourceParent);
    IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
    TaxonomyNode *segParent = sourceSeg->taxonomy();
    int row = segParent->getSubElements().size() + m_taxonomySegs[segParent].indexOf(sourceSeg);
    QModelIndex proxyIndex = mapFromSource(sourceIndex);
    beginRemoveRows(proxyIndex.parent(),row,row);
  } else // In case sourceParent is taxonomyRoot, proxyParent will be an invalid index
  {
      beginRemoveRows(mapFromSource(sourceParent), start, end);
  }
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
    
  if (sourceParent == model->sampleRoot())
    return;
  
  // If we added new segmentations we have to update our segmentation map
  if (sourceParent == model->segmentationRoot())
    updateSegmentations();
  
  //Finally, notify views we removed requested rows
  endRemoveRows();
}

//------------------------------------------------------------------------
void TaxonomyProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  if (!model)
    return;
  
  const QModelIndex proxyTopLeft = mapFromSource(sourceTopLeft);
  const QModelIndex proxyBottomRight = mapFromSource(sourceBottomRight);
  
  IModelItem *segItem = static_cast<IModelItem *>(proxyTopLeft.internalPointer());
  assert(segItem);
  Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
  if (seg);
  {
    foreach(const TaxonomyNode *tax, m_taxonomySegs.keys())
    {
      int row = m_taxonomySegs[tax].indexOf(seg);
      if (row >= 0)
      {
	QModelIndex taxIndex = mapFromSource(model->taxonomyIndex(const_cast<TaxonomyNode*>(tax)));
	beginRemoveRows(taxIndex,row,row);
      }
    }
  }
  
  updateSegmentations();
  
  if (seg)
  {
    foreach(const TaxonomyNode *tax, m_taxonomySegs.keys())
    {
      int row = m_taxonomySegs[tax].indexOf(seg);
      if (row >= 0)
      {
	QModelIndex taxIndex = mapFromSource(model->taxonomyIndex(const_cast<TaxonomyNode *>(tax)));
	beginInsertRows(taxIndex,row,row);
      }
    }
  }
  emit dataChanged(proxyTopLeft, proxyBottomRight);
  if (proxyTopLeft.isValid())
  {
    emit dataChanged(proxyTopLeft.parent(),proxyTopLeft.parent());
    emit dataChanged(proxyTopLeft, proxyBottomRight);
  }
  
  if (seg)
  {
    endInsertRows();
    endRemoveRows();
  }
}


//------------------------------------------------------------------------
void TaxonomyProxy::updateSegmentations() const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  if (!model)
    return;

  m_taxonomySegs.clear();
  int rows = model->rowCount(model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = model->index(row, 0, model->segmentationRoot());
    IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
    assert(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    assert(seg);
    m_taxonomySegs[seg->taxonomy()].push_back(seg);
  }
}





