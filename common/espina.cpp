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
  
  if (index.internalId() < 3)
  {
    return "Falso";
  }
  
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
  if (!parent.isValid())
    return 3;
  
  if (parent.internalPointer() == taxonomyRoot().internalPointer())
    return numOfSubTaxonomies(m_tax);
  
  if (parent.internalId() == sampleRoot().internalId())
    return m_samples.size();
  
  if (parent.internalId() == segmentationRoot().internalId())
    return m_segmentations.size();
  
  // Cast to base type 
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Check if Taxonomy Item
  TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(parentItem);
  if (taxItem)
    return numOfSubTaxonomies(taxItem);// + numOfSegmentations(taxItem);
  // Otherwise Samples and Segmentations have no children
  return 0;
}

//------------------------------------------------------------------------
QModelIndex EspINA::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  if ( child.internalPointer() == taxonomyRoot().internalPointer() 
    || child.internalId() == sampleRoot().internalId() 
    || child.internalId() == segmentationRoot().internalId())
    return QModelIndex();
  
  IModelItem *childItem = static_cast<IModelItem *>(child.internalPointer());
  assert (childItem);
  
  // Checks if Taxonomy
  TaxonomyNode *childNode = dynamic_cast<TaxonomyNode *>(childItem);
  if (childNode)
  {
    TaxonomyNode *parentNode = m_tax->getParent(childNode->getName());
    return index(parentNode);
  }
  // Checks if Sample
  Sample *parentSample = dynamic_cast<Sample *>(childItem);
  if (parentSample)
    return sampleRoot();
  
  // Otherwise is a segmentation
  Segmentation *childProduct = dynamic_cast<Segmentation *>(childItem);
  if (childProduct)
    return segmentationRoot();
  
  assert(false);
  return QModelIndex();
}

//------------------------------------------------------------------------
//! Returned index is compossed by the row, column and an element).
QModelIndex EspINA::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  if (!parent.isValid())
  {
    assert(row<3);
    if (row == 0)
      return taxonomyRoot();
    if (row == 1)
      return sampleRoot();
    if (row == 2)
      return segmentationRoot();
  }
  
  // Checks if parent is Sample's root
  if (parent.internalPointer() == sampleRoot().internalPointer())
  {
    IModelItem * element = m_samples[row];
    return createIndex(row,column,element);
  }
  
  // Checks if parent is Segmentation's root
  if (parent.internalPointer() == segmentationRoot().internalPointer())
  {
    IModelItem * element = m_segmentations[row];
    return createIndex(row,column,element);
  }
  
  // Segmentation can't be parent index
  IModelItem *parentItem = static_cast<IModelItem *>(parent.internalPointer());
  // Checks if parent is Taxonomy
  TaxonomyNode *taxItem = dynamic_cast<TaxonomyNode *>(parentItem);
  if (taxItem)
  {
    IModelItem *element;
    int subTaxonomies = numOfSubTaxonomies(taxItem);
    if (row < subTaxonomies)
    {
      element = taxItem->getSubElements()[row];
      //std::cout << taxItem->getSubElements()[row]->getName().toStdString();
    }
    else
      element = segmentations(taxItem)[row-subTaxonomies];
   
    return createIndex(row,column,element);
  }
  // Otherwise, invalid index
  assert(false);
  return QModelIndex();
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
QModelIndex EspINA::taxonomyRoot() const
{

  return createIndex(0,0,m_tax);
}

//------------------------------------------------------------------------
QModelIndex EspINA::sampleRoot() const
{
  return createIndex(1,0,1);
}

//------------------------------------------------------------------------
QModelIndex EspINA::segmentationRoot() const
{
  return createIndex(2,0,2);
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
    return taxonomyRoot();
  
  TaxonomyNode *parentNode = m_tax->getParent(node->getName());
  assert(parentNode);
  int row = parentNode->getSubElements().indexOf(node);
  // Returns the row and column of node inside parentNode, and the node itself
  return createIndex(row,0,node);
}





//------------------------------------------------------------------------
QList<Segmentation * > EspINA::segmentations(const TaxonomyNode* taxonomy, bool recursive) const
{
  // Get all segmentations that belong to taxonomy
  QList<Segmentation *> segs;
  
  segs.append(m_taxonomySegs[taxonomy]);
  
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
void EspINA::addSample(Sample* sample)
{
  Cache *cache = Cache::instance();
  cache->insert(sample->id(),sample->data());
  
  m_activeSample = sample;
  m_samples.push_back(sample);
}


//------------------------------------------------------------------------
void EspINA::addSegmentation(Segmentation *seg)
{
  TaxonomyNode *node = m_newSegType;
  
  // We need to notify other components that the model has changed
  QModelIndex parent = index(node);
  int lastRow = rowCount(parent);
  
  beginInsertRows(parent,lastRow,lastRow);
  seg->setTaxonomy(node);
  m_taxonomySegs[m_newSegType].push_back(seg);
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
  m_newSegType = node;//->getName();//item->data(Qt::DisplayRole).toString();
}


//------------------------------------------------------------------------
EspINA::EspINA(QObject* parent)
: QAbstractItemModel(parent)
, m_activeSample(NULL)
{
  loadTaxonomy();
  m_newSegType = m_tax->getComponent("Symetric");
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

