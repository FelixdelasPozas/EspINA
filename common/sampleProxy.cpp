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


#include "sampleProxy.h"

#include "espina.h"
#include "sample.h"
#include "segmentation.h"

// libCajal
#include <data/modelItem.h>

//Debug
#include "espina_debug.h"

SampleProxy::SampleProxy(QObject* parent)
    : QAbstractProxyModel(parent)
{
  updateSegmentations();
}

SampleProxy::~SampleProxy()
{

}

//------------------------------------------------------------------------
void SampleProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  QAbstractProxyModel::setSourceModel(sourceModel);
  updateSegmentations();
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsInserted(const QModelIndex&, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
          this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(sourceDataChanged(const QModelIndex &,const QModelIndex &)));
}

//------------------------------------------------------------------------
int SampleProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  //updateSegmentations();

  if (!parent.isValid())
    return model->rowCount(model->sampleRoot());

  // Cast to base type
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  Sample *parentSample = dynamic_cast<Sample *>(parentItem);
  if (parentSample)
  {
    int numSegs = m_sampleSegs[parentSample].size();
    return numSegs;//parentTax->getSubElements().size() + numSegs;
  }
  // Otherwise Segmentations have no children in the model
  return 0;
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (!hasIndex(row, column, parent))
    return QModelIndex();
  
  if (!parent.isValid())
  {
    return mapFromSource(model->index(row, column, model->sampleRoot()));
    //NOTE: Old method
//     QModelIndex sourceIndex = model->index(row, column, model->sampleRoot());
//     IModelItem * interalPtr = static_cast<IModelItem *>(sourceIndex.internalPointer());
//     Sample * sample = dynamic_cast<Sample *>(interalPtr); //DEBUG
//     assert(sample); //DEBUG
//     return createIndex(row, column, interalPtr);
  }
  //NOTE: We don't use mapFromSource to reduce computational load
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  Sample * parentSample = dynamic_cast<Sample *>(parentItem);
  assert(parentSample);
  IModelItem *internalPtr = m_sampleSegs[parentSample][row];
  assert(internalPtr);
  return createIndex(row, column, internalPtr);
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);

  if (!child.isValid())
    return QModelIndex();

  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert(childItem);
  // Checks if Sample
  Sample *childSample = dynamic_cast<Sample *>(childItem);
  if (childSample)
    return QModelIndex();

  // Otherwise is a segmentation
  Segmentation *childSeg = dynamic_cast<Segmentation *>(childItem);
  if (childSeg)
    return mapFromSource(model->sampleIndex(childSeg->origin()));

  assert(false);
  return QModelIndex();
}
//------------------------------------------------------------------------
QModelIndex SampleProxy::mapFromSource(const QModelIndex& sourceIndex) const
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
   //There are no taxonomies in the new model
   TaxonomyNode * tax = dynamic_cast<TaxonomyNode *>(sourceItem);
   if (tax)
     return QModelIndex();
   
   //Samples are shown in the same order than in the original model
   Sample *sample = dynamic_cast<Sample *>(sourceItem);
   if (sample)
     return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
   
   //Segmentations
   Segmentation *seg = dynamic_cast<Segmentation *>(sourceItem);
   if (seg)
   {
     Sample *parent = seg->origin();
     int row = m_sampleSegs[parent].indexOf(seg);
     return createIndex(row,0,sourceIndex.internalPointer());
   }
   
   assert(false);
   return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex SampleProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();

  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  IModelItem *proxyItem = static_cast<IModelItem *>(proxyIndex.internalPointer());
  assert(proxyItem);
  
  Sample *proxySample = dynamic_cast<Sample *>(proxyItem);
  if (proxySample)
    return model->sampleIndex(proxySample);
  
  Segmentation *proxySeg = dynamic_cast<Segmentation *>(proxyItem);
  if (proxySeg)
    return model->segmentationIndex(proxySeg);

  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
void SampleProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
  {
    beginInsertRows(mapFromSource(model->sampleRoot()), start, end);
    endInsertRows();
  }else if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    
    QModelIndex sourceIndex = model->index(start, 0, sourceParent);
    IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
    assert(sourceSeg);
    Sample *segParent = sourceSeg->origin();
    QModelIndex parentIndex = mapFromSource(model->sampleIndex(segParent));
    int row = m_sampleSegs[segParent].indexOf(sourceSeg);
    beginInsertRows(parentIndex, row, row);
    endInsertRows();
  }
}

//------------------------------------------------------------------------
void SampleProxy::sourceRowsAboutToBeRemoved(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
  {
    beginRemoveRows(mapFromSource(sourceParent),start,end);
  } else if (sourceParent == model->segmentationRoot())
  {
    assert(start == end);//TODO: Add support for multiple deletions
    // Need to find its parent before deletion
    QModelIndex sourceIndex = model->index(start, 0, sourceParent);
    IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
    Sample *segParent = sourceSeg->origin();
    int row = m_sampleSegs[segParent].indexOf(sourceSeg);
    QModelIndex proxyIndex = mapFromSource(sourceIndex);
    beginRemoveRows(proxyIndex.parent(),row,row);
  }
}


//------------------------------------------------------------------------
void SampleProxy::sourceRowsRemoved(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (sourceParent == model->sampleRoot())
    endRemoveRows();
  
  if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    endRemoveRows();
  }
}

void SampleProxy::sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight)
{
  const QModelIndex proxyTopLeft = mapFromSource(sourceTopLeft);
  const QModelIndex proxyBottomRight = mapFromSource(sourceBottomRight);
  
  emit dataChanged(proxyTopLeft, proxyBottomRight);
}



//------------------------------------------------------------------------
void SampleProxy::updateSegmentations() const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  if (!model)
    return;

  m_sampleSegs.clear();
  int rows = model->rowCount(model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = model->index(row, 0, model->segmentationRoot());
    IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
    assert(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    assert(seg);
    m_sampleSegs[seg->origin()].push_back(seg);
  }
}
