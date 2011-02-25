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
#include <data/modelItem.h>
#include "traceNodes.h"

//Debug
#include <assert.h>
#include <QDebug>
#include <iostream>

QModelIndex SampleProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();
  
  return createIndex(sourceIndex.row(),sourceIndex.column(),sourceIndex.internalPointer());
}

QModelIndex SampleProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();
  
  return sourceModel()->index(proxyIndex.row(),proxyIndex.column(),proxyIndex.parent());
}

int SampleProxy::columnCount(const QModelIndex& parent) const
{
  return 1;
}

int SampleProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  updateSegmentations();
  
  if (!parent.isValid())
    return 3;
  
  if (parent == mapFromSource(model->taxonomyRoot()))
    return 0;
  
  if (parent == mapFromSource(model->sampleRoot()))
    return model->rowCount(model->sampleRoot());
  
  if (parent == mapFromSource(model->segmentationRoot()))
    return 0;
  
  // Cast to base type 
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Check if Taxonomy Item
  Sample *parentSample = dynamic_cast<Sample *>(parentItem);
  if (parentSample)
  {
    int numSegs = m_sampleSegs[parentSample].size();
    return numSegs;//parentTax->getSubElements().size() + numSegs;
  }
  // Otherwise Samples and Segmentations have no children
  return 0;
}

QModelIndex SampleProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);

  if (!child.isValid())
    return QModelIndex();
  
  if ( child == mapFromSource(model->taxonomyRoot()) 
    || child == mapFromSource(model->sampleRoot()) 
    || child == mapFromSource(model->segmentationRoot()))
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  // Checks if Sample
  Sample *childSample = dynamic_cast<Sample *>(childItem);
  if (childSample)
    return mapFromSource(model->sampleRoot());
  
  // Otherwise is a segmentation
  Segmentation *childSeg = dynamic_cast<Segmentation *>(childItem);
  if (childSeg)
  {
    QModelIndex p = mapFromSource(model->sampleIndex(childSeg->origin()));
    
    IModelItem * element = static_cast<IModelItem *>(p.internalPointer());
    Sample * sample = dynamic_cast<Sample *>(element);
    return p;
  }
  
  assert(false);
  return QModelIndex();
}

QModelIndex SampleProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());

  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  if (!parent.isValid())
  {
    assert(row<3);
    if (row == 0)
      return mapFromSource(model->taxonomyRoot());
    if (row == 1)
      return mapFromSource(model->sampleRoot());
    if (row == 2)
      return mapFromSource(model->segmentationRoot());
  }
  
  if (parent == mapFromSource(model->sampleRoot()))
  {
    QModelIndex sourceIndex = model->index(row,column,parent);
    IModelItem * element = static_cast<IModelItem *>(sourceIndex.internalPointer());
    Sample * sample = dynamic_cast<Sample *>(element);
    //std::cout << "Name:" << sample->name.toStdString() << std::endl;
    assert(sample);
    return createIndex(row,column,element);
  }
  else 
  {
    IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
    Sample * parentSample = dynamic_cast<Sample *>(parentItem);
    assert(parentSample);
    Segmentation *seg = m_sampleSegs[parentSample][row];
    IModelItem *indexItem = seg;
    return createIndex(row,column,indexItem);
    
  }
  
  // Otherwise, invalid index
  assert(false);
  return QModelIndex();
}

void SampleProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
    QAbstractProxyModel::setSourceModel(sourceModel);
    updateSegmentations();
    connect(sourceModel,SIGNAL(rowsInserted(const QModelIndex&,int,int)),
	    this,SLOT(sourceRowsInserted(const QModelIndex&,int,int)));
}

void SampleProxy::sourceRowsInserted(const QModelIndex& sourceParent, int start, int end)
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  if (sourceParent == model->segmentationRoot())
  {
    updateSegmentations();
    reset();
    return;
    //Look for modified segmentations
    for (int r = start; r <= end; r++)
    {

      QModelIndex sourceIndex = model->index(r,0,sourceParent);
      IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
      Segmentation *sourceSeg = dynamic_cast<Segmentation *>(sourceItem);
      assert(sourceSeg);
      Sample *segSample = sourceSeg->origin();
      QModelIndex sampleIndex = mapFromSource(model->sampleIndex(segSample));
      int end = rowCount(sampleIndex);
      beginInsertRows(sampleIndex,end,end);
      endInsertRows();
    }
  }
}

void SampleProxy::updateSegmentations() const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  if (!model)
    return;
  
  m_sampleSegs.clear();
  int rows = model->rowCount(model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = model->index(row,0,model->segmentationRoot());
    IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
    assert(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    assert(seg);
    m_sampleSegs[seg->origin()].push_back(seg);
  }
}

