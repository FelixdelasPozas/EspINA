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

int TaxonomyProxy::rowCount(const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  //updateSegmentations();
  
  if (!parent.isValid())
    return 3;
  
  if (parent.internalPointer() == model->taxonomyRoot().internalPointer())
    return 2;//Num sub tax
  
  if (parent.internalId() == model->sampleRoot().internalId())
    return 0;
  
  if (parent.internalId() == model->segmentationRoot().internalId())
    return 0;
  
  // Cast to base type 
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Check if Taxonomy Item
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    std::cout << "Getting rows of " << parentTax->getName().toStdString() << std::endl;
    int numSegs = 0;//m_taxonomySegs[taxItem].size();
    //std::cout << taxItem->getName().toStdString() << ": " << numSegs << " segmentations" << std::endl; 
    return parentTax->getSubElements().size() + numSegs;
  }
    //return numOfSubTaxonomies(taxItem);// + numOfSegmentations(taxItem);
  // Otherwise Samples and Segmentations have no children
  std::cout << "Getting rows of invalid parent" << std::endl;
  
  return 0;
}

QModelIndex TaxonomyProxy::index(int row, int column, const QModelIndex& parent) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  if (!parent.isValid())
  {
    assert(row<3);
    if (row == 0)
      return createIndex(0,0,model->taxonomyRoot().internalPointer());
    if (row == 1)
      return model->sampleRoot();
    if (row == 2)
      return model->segmentationRoot();
  }
  
  // Segmentation can't be parent index
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Checks if parent is Taxonomy
  TaxonomyNode *parentTax = dynamic_cast<TaxonomyNode *>(parentItem);
  if (parentTax)
  {
    IModelItem *element;
    //int subTaxonomies = numOfSubTaxonomies(taxItem);
    int subTaxonomies = parentTax->getSubElements().size();
    if (row < subTaxonomies)
    {
      TaxonomyNode *taxItem = parentTax->getSubElements()[row];
      //int numSegs = m_taxonomySegs[taxItem].size();
      //std::cout << taxItem->getName().toStdString() << ": " << numSegs << " segmentations" << std::endl;
      element = taxItem;
    }
    else
      assert(false);
      //element = segmentations(taxItem)[row-subTaxonomies];
   
    return createIndex(row,column,element);
  }
  // Otherwise, invalid index
  assert(false);
  return QModelIndex();
}

QModelIndex TaxonomyProxy::parent(const QModelIndex& child) const
{
  EspINA *model = dynamic_cast<EspINA *>(sourceModel());
  assert(model);
  
  if (!child.isValid())
    return QModelIndex();
  
  if ( child.internalPointer() == model->taxonomyRoot().internalPointer() 
    || child.internalId() == model->sampleRoot().internalId() 
    || child.internalId() == model->segmentationRoot().internalId())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  
  // Checks if Taxonomy
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    std::cout << "Getting parent of " << childNode->getName().toStdString() << std::endl;
    TaxonomyNode *tax = static_cast<TaxonomyNode *>(model->taxonomyRoot().internalPointer());
    TaxonomyNode *parentNode = tax->getParent(childNode->getName());
    std::cout << "\tParent is " << parentNode->getName().toStdString() << std::endl;
    if (parentNode == tax)
      return createIndex(0,0,tax);
    TaxonomyNode *gparentNode = tax->getParent(parentNode->getName());
    int r = gparentNode->getSubElements().indexOf(parentNode);
    return createIndex(r,0,parentNode);//childNode
  }
  
  // Otherwise is a segmentation
  Segmentation *childProduct = dynamic_cast<Segmentation *>(childItem);
  if (childProduct)
    assert(false);
    //return createIndex(model->segmentationRoot().row(),model->segmentationRoot().column(),model->segmentationRoot().internalId());
  
  assert(false);
  return QModelIndex();
}

QModelIndex TaxonomyProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return QModelIndex();
  
  return createIndex(sourceIndex.row(),sourceIndex.column(),sourceIndex.internalPointer());
  
  //IModelItem *item = static_cast<IModelItem *>(sourceIndex.internalPointer());
  //TaxonomyNode *node =  dynamic_cast<TaxonomyNode *>(item);
  //if (node)// && node->getName() == "Synapse")
    //std::cout << node->getName().toStdString() << std::endl;
  QModelIndex sourceParent = sourceIndex.parent();
  QModelIndex proxyParent = mapFromSource(sourceParent);
  //std::cout << "source parent: (" << sourceParent.row() << "," << sourceParent.column() << "," << sourceParent.internalPointer() << ") >> proxy parent: (" << proxyParent.row() << "," << proxyParent.column() << "," << proxyParent.internalPointer() << std::endl; 
  return this->index(sourceIndex.row(),sourceIndex.column(),proxyParent);
  //return createIndex(sourceIndex.row(),sourceIndex.column(),sourceIndex.internalPointer());
}

QModelIndex TaxonomyProxy::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid())
    return QModelIndex();
  
  //QModelIndex proxyParent = proxyIndex.parent();
  //QModelIndex sourceParent = mapToSource(proxyParent);
  QModelIndex sourceIndex = sourceModel()->index(proxyIndex.row(),proxyIndex.column(),proxyIndex.parent());
  //IModelItem *item = static_cast<IModelItem *>(sourceIndex.internalPointer());
  //TaxonomyNode *node =  dynamic_cast<TaxonomyNode *>(item);
  //if (node)// && node->getName() == "Synapse")
    //std::cout << node->getName().toStdString() << std::endl;
  return sourceIndex;
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





