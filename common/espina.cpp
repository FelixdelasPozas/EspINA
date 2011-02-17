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

int EspINA::columnCount(const QModelIndex& parent) const
{
  return 1;
}

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
  if (!parentNode)
    return 0;
  
  // Taxonomy leaf nodes can hold segmentations
    if (isLeaf(parentNode))
    {
      // Look for segmentations belonging to this  taxonomical class
      return segmentations(parentNode).size();
    }
    else
      return parentNode->getSubElements().size();
}

QModelIndex EspINA::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    if (childNode->getName() == m_tax->getName())
      return QModelIndex();
  
    TaxonomyNode *parentNode = m_tax->getParent(childNode->getName());
    assert(parentNode);
    int row = parentNode->getSubElements().indexOf(childNode);
  // Returns the row and column of parentNode in the parentNode's parent
    return createIndex(row,0,parentNode);
  }
  Product *childProduct = dynamic_cast<Product *>(childItem);
  if (childProduct)
  {
    int row = segmentations(childProduct->taxonomy()).indexOf(childProduct);
    return createIndex(row,0,childProduct->taxonomy());
  }
  
  return QModelIndex();
  
  
  /*
  // We avoid setting the Taxonomy descriptor as parent of an index
  if (childParentNode->getName() == "FEM")
    return QModelIndex();
  
  TaxonomyNode *grandParentNode = m_tax->getParent(childParentNode->getName());
  assert(grandParentNode);
  int row = grandParentNode->getSubElements().indexOf(childParentNode);
  // Returns the row and column of parentNode in the parentNode's parent
  return createIndex(row,0,grandParentNode);
  */
}


//------------------------------------------------------------------------
////! Returned index is compossed by the row, column and parent element).
////! Thus, we don't need to store leaf nodes (which in our case can be
////! either a taxonomy or a segmentation)
//! Returned index is compossed by the row, column and an element).
QModelIndex EspINA::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  TaxonomyNode *parentNode;
  if (!parent.isValid())
  {// It corresponds to a taxonomy
  parentNode = m_tax;
  }
  else
  {
    //parentNode = indexNode(parent);
    // Segmentation can't be parent index
    parentNode = static_cast<TaxonomyNode *>(parent.internalPointer());
  }
  assert(parentNode);
  
  //!COMMENT// Both taxonomy and segmentation parents are Taxonomy nodes
  //return createIndex(row,column,parentNode);
  IModelItem *element;
  if (isLeaf(parentNode))
  {
    element = segmentations(parentNode)[row];
  }
  else
  {
    element = parentNode->getSubElements()[row];
  }
  return createIndex(row,column,element);
}

//------------------------------------------------------------------------
QVariant EspINA::headerData(int section, Qt::Orientation orientation, int role) const
{
  // if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  // return "Segmented Objects";
  return QVariant();
}

//------------------------------------------------------------------------
Qt::ItemFlags EspINA::flags(const QModelIndex& index) const
{
  return QAbstractItemModel::flags(index);
}

//------------------------------------------------------------------------
TaxonomyNode* EspINA::indexNode(const QModelIndex& index) const
{
  TaxonomyNode *parentNode  = static_cast<TaxonomyNode *>(index.internalPointer());
  if (isLeaf(parentNode))
    return NULL; // This is a segmentation
  else 
    return parentNode->getSubElements()[index.row()];
}


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
  // Returns the row and column of parentNode in the parentNode's parent
  return createIndex(row,0,parentNode);
}





//------------------------------------------------------------------------
QList< Product* > EspINA::segmentations(const TaxonomyNode* taxonomy) const
{
  // Get all segmentations that belong to taxonomy
  QList<Product *> segs;
  
  segs.append(m_segmentations[taxonomy->getName()]);
  
  // Get all segmentations that belong to taxonomy's children
  TaxonomyNode *child;
  foreach(child,taxonomy->getSubElements())
  {
    segs.append(segmentations(child));
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
  QModelIndex nodeIndex = index(node);
  int lastRow = rowCount(nodeIndex);
  
  beginInsertRows(nodeIndex,lastRow,lastRow);
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
  m_newSegType = item->data(Qt::DisplayRole).toString();
  /*
  TaxonomyNode *node = indexNode(index);
  if (!node)
    node = static_cast<TaxonomyNode *>(index.internalPointer());
  m_newSegType = node->getName();
  */
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
  m_tax->print();
}

