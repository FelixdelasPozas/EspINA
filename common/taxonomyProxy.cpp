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

#include <data/modelItem.h>
#include <data/taxonomy.h>
#include "espina.h"
#include "traceNodes.h"


#include <assert.h>
#include <QDebug>
#include <../../../InsightToolkit-3.20.0/Utilities/vxl/vcl/iso/vcl_iostream.h>
//------------------------------------------------------------------------
TaxonomyProxy::TaxonomyProxy(QObject* parent)
: QSortFilterProxyModel(parent)
{

}

//------------------------------------------------------------------------
TaxonomyProxy::~TaxonomyProxy()
{
}

//------------------------------------------------------------------------
int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  //TODO: Do only when model changes
  updateSegmentations();
  
  if (!parent.isValid())
  {
    std::cout << "Num Hijos[Root] = " << model->rowCount(model->taxonomyRoot()) << std::endl;
    return model->rowCount(model->taxonomyRoot());
  }
    
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  assert(parentItem);
  TaxonomyNode *parentTax = static_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    int numSegs = m_taxonomySegs[parentTax].size();
    std::cout << "Num Hijos[" << parentTax->getName().toStdString() << "] = " << parentTax->getSubElements().size() << std::endl;// model->rowCount(mapToSource(parent)) << std::endl;
    return parentTax->getSubElements().size();
  }
  else
    return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  
  if (!child.isValid())
    return QModelIndex();
  
  if (child.internalPointer() == model->taxonomyRoot().internalPointer())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert(childItem);
  TaxonomyNode *childTax = dynamic_cast<TaxonomyNode *>(childItem);
  if (childTax) 
  {
    return QModelIndex();
    QModelIndex parent = sourceModel()->parent(mapToSource(child));
    return parent;
  }
  
  Segmentation *childSeg = dynamic_cast<Segmentation *>(childItem);
  if (childSeg)
  {
    IModelItem *tax = childSeg->taxonomy();
    return createIndex(0,0,tax);
  }
  return QModelIndex();
}

//------------------------------------------------------------------------
QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  if (!parent.isValid())
    return createIndex(0,0,model->taxonomyRoot().internalPointer());
  
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  assert(parentItem);
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  assert(parentTax);
  int taxRows = parentTax->getSubElements().size();
  if (row < taxRows)
  {
    IModelItem *tax = parentTax->getSubElements()[row];
    return createIndex(row,column,tax);
  }
  else
  {
    IModelItem *seg = m_taxonomySegs[parentTax][row-taxRows];
    return createIndex(row,column,seg);
  }
}

QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  
  if (sourceIndex == model->taxonomyRoot())
    return createIndex(0,0,model->taxonomyRoot().internalPointer());
  
  IModelItem *sourceItem = static_cast<IModelItem *>(sourceIndex.internalPointer());
  assert(sourceItem);
  TaxonomyNode *sourceTax = dynamic_cast<TaxonomyNode *>(sourceItem);
  if (sourceTax)
    return index(sourceIndex.row(),sourceIndex.column(),sourceIndex.parent());
  else
    return QModelIndex();
}

QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();
  return sourceModel()->index(proxyIndex.row(),proxyIndex.column(),proxyIndex.parent());
    //return QSortFilterProxyModel::mapToSource(proxyIndex);
}


//------------------------------------------------------------------------
void TaxonomyProxy::updateSegmentations() const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  
  m_taxonomySegs.clear();
  int rows = model->rowCount(model->segmentationRoot());
  for (int row = 0; row < rows; row++)
  {
    QModelIndex segIndex = model->index(row,0,model->segmentationRoot());
    IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
    assert(segItem);
    Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
    assert(seg);
    m_taxonomySegs[seg->taxonomy()].push_back(seg);
  }
}





