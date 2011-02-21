/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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


#include "espina.h"
//Espina
#include <data/taxonomy.h>
#include "traceNodes.h"
#include "cache/cache.h"

#include <QDebug>
#include <iostream>
//------------------------------------------------------------------------
EspINA *EspINA::m_singleton(NULL);

//------------------------------------------------------------------------
EspINA* EspINA::instance()
{
  if (!m_singleton)
    m_singleton = new EspINA();
  
  return m_singleton;
}

//------------------------------------------------------------------------
EspINA::~EspINA()
{
  
}


QVariant EspINA::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  
  IModelItem *indexItem = static_cast<IModelItem *>(index.internalPointer());
  
  return indexItem->data(role);
  
  
  //bool isSegmentation = isLeaf(indexNode);
/*  
  switch (role)
  {
    case Qt::DisplayRole:
	return indexNode->getName();
    case Qt::DecorationRole:
	return indexNode->getColor();
    default:
      return QVariant();
  }*/
}

//------------------------------------------------------------------------
int EspINA::columnCount(const QModelIndex& parent) const
{
  return 1;
}

//------------------------------------------------------------------------
int EspINA::rowCount(const QModelIndex& parent) const
{
  // This avoid creating indexes of an unitialized model
  TaxonomyNode *parentNode;
  if (!parent.isValid())
    parentNode = m_tax;
  else
  {
    //parentNode = indexNode(parent);
    IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
    parentNode = dynamic_cast<TaxonomyNode *>(parentItem);
  }
  // Segmentations haven't children
  if (!parentNode)
    return 0;
  
  return numOfSubTaxonomies(parentNode) + numOfSegmentations(parentNode);
}

//------------------------------------------------------------------------
QModelIndex EspINA::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  
  // Model Item can be either Taxonomy Nodes or Segmentation Nodes
  TaxonomyNode *parentNode;
  
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    parentNode = m_tax->getParent(childNode->getName());
  }
  else
  {
    Product *childProduct = dynamic_cast<Product *>(childItem);
    assert(childProduct);
    parentNode = childProduct->taxonomy();
  }
  
  return index(parentNode);
}


//------------------------------------------------------------------------
//! Returned index is compossed by the row, column and an element).
QModelIndex EspINA::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  TaxonomyNode *parentNode;
  if (!parent.isValid())
  { // It corresponds to a taxonomy
    parentNode = m_tax;
  }
  else
  { // Segmentation can't be parent index
    IModelItem *item = static_cast<IModelItem *>(parent.internalPointer());
    parentNode       = dynamic_cast<TaxonomyNode *>(item);
  }
  assert(parentNode);
  
  IModelItem *element;
  int subTaxonomies = numOfSubTaxonomies(parentNode);
  if (row < subTaxonomies)
    element = parentNode->getSubElements()[row];
  else
    element = segmentations(parentNode)[row-subTaxonomies];
  
  return createIndex(row,column,element);
}

//------------------------------------------------------------------------
int EspINA::numOfSubTaxonomies(TaxonomyNode* tax) const
{
  return tax->getSubElements().size();
}

//------------------------------------------------------------------------
int EspINA::numOfSegmentations(TaxonomyNode* tax) const
{
  return segmentations(tax).size();
}


//------------------------------------------------------------------------
QVariant EspINA::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant();
}

//------------------------------------------------------------------------
Qt::ItemFlags EspINA::flags(const QModelIndex& index) const
{
  return QAbstractItemModel::flags(index);
}

//------------------------------------------------------------------------
/*
TaxonomyNode* EspINA::indexNode(const QModelIndex& index) const
{
  TaxonomyNode *parentNode  = static_cast<TaxonomyNode *>(index.internalPointer());
  if (isLeaf(parentNode))
    return NULL; // This is a segmentation
  else 
    return parentNode->getSubElements()[index.row()];
}*/


//------------------------------------------------------------------------
bool EspINA::isLeaf(TaxonomyNode* node) const
{
  return node->getSubElements().size() == 0;
}

//------------------------------------------------------------------------
QModelIndex EspINA::index(TaxonomyNode* node) const
{
  // We avoid setting the Taxonomy descriptor as parent of an index
  if (node->getName() == m_tax->getName())
    return QModelIndex();
  
  TaxonomyNode *parentNode = m_tax->getParent(node->getName());
  assert(parentNode);
  int row = parentNode->getSubElements().indexOf(node);
  // Returns the row and column of node inside parentNode, and the node itself
  return createIndex(row,0,node);
}





//------------------------------------------------------------------------
QList< Product* > EspINA::segmentations(const TaxonomyNode* taxonomy, bool recursive) const
{
  // Get all segmentations that belong to taxonomy
  QList<Product *> segs;
  
  segs.append(m_segmentations[taxonomy->getName()]);
  
  if (recursive)
  {
    // Get all segmentations that belong to taxonomy's children
    TaxonomyNode *child;
    foreach(child,taxonomy->getSubElements())
    {
      segs.append(segmentations(child,recursive));
    }
  }
  return segs;
}

//------------------------------------------------------------------------
void EspINA::addSample(Product* sample)
{
  Cache *cache = Cache::instance();
  cache->insert(sample->id(),sample->data());
  
  m_samples.push_back(sample);
}


//------------------------------------------------------------------------
void EspINA::addSegmentation(Product* seg)
{
  TaxonomyNode *node = m_tax->getComponent(m_newSegType);
  
  // We need to notify other components that the model has changed
  QModelIndex parent = index(node);
  int lastRow = rowCount(parent);
  
  beginInsertRows(parent,lastRow,lastRow);
  seg->setTaxonomy(node);
  m_segmentations[m_newSegType].push_back(seg);
  endInsertRows();
  
  emit render(seg);
  emit sliceRender(seg);
}

//------------------------------------------------------------------------
void EspINA::setUserDefindedTaxonomy(const QModelIndex& index)
{
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  TaxonomyNode *node = dynamic_cast<TaxonomyNode *>(item);
  if (!node)
  {
    Product *seg = dynamic_cast<Product *>(item);
    if (seg)
      node = seg->taxonomy();
  }
  assert(node);
  m_newSegType = node->getName();//item->data(Qt::DisplayRole).toString();
}


//------------------------------------------------------------------------
EspINA::EspINA(QObject* parent): QAbstractItemModel(parent)
{
  loadTaxonomy();
  m_newSegType = "Symetric";
  m_analysis = new ProcessingTrace();
}

//------------------------------------------------------------------------
void EspINA::loadTaxonomy()
{
  m_tax = new TaxonomyNode("FEM");
  TaxonomyNode *newNode;
  newNode = m_tax->addElement("Synapse","FEM");
  newNode->setColor(QColor(255,0,0));
  m_tax->addElement("Vesicles","FEM");
  m_tax->addElement("Symetric","Synapse");
  newNode = m_tax->addElement("Asymetric","Synapse");
  newNode->setColor(QColor(Qt::yellow));
  /* // DEBUG
  m_tax->addElement("A","Vesicles");
  m_tax->addElement("B","Vesicles");
  m_tax->addElement("B1","B");
  m_tax->addElement("B2","B");
  m_tax->addElement("B21","B2");
  m_tax->addElement("B22","B2");
  m_tax->addElement("B23","B2");
  m_tax->addElement("B24","B2");
  m_tax->addElement("B3","B");
  m_tax->addElement("C","Vesicles");
  m_tax->print();
  */
}

