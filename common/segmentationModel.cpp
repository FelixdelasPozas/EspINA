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


#include "segmentationModel.h"

// ESPINA
#include <data/taxonomy.h>
#include "objectManager.h"

// Qt
#include <QColor>

// Debug
#include <QDebug>
#include <assert.h>
#include <iostream>

QVariant SegmentationModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  TaxonomyNode *indexParentNode = static_cast<TaxonomyNode *>(index.internalPointer());
  
  bool isSegmentation = isLeaf(indexParentNode);
  
  switch (role)
  {
    case Qt::DisplayRole:
      if (isSegmentation)
	return "Segmentation";
      else 
	return indexNode(index)->getName();
    case Qt::DecorationRole:
      if (isSegmentation)
	return indexParentNode->getColor();
      else
	return indexNode(index)->getColor();
    default:
      return QVariant();
  }
}

int SegmentationModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}

int SegmentationModel::rowCount(const QModelIndex& parent) const
{
  // This avoid creating indexes of an unitialized model
  if (!m_tax || !m_om)
    return 0;
  
  TaxonomyNode *parentNode;
  if (!parent.isValid())
    parentNode = m_tax;
  else
    parentNode = indexNode(parent);
  
  if (!parentNode)
    return 0;
  
  // Taxonomy leaf nodes can hold segmentations
  if (isLeaf(parentNode))
  {
    // Look for segmentations belonging to this  taxonomical class
    return m_om->segmentations(parentNode->getName()).size();
  }
  else
    return parentNode->getSubElements().size();
}

QModelIndex SegmentationModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  TaxonomyNode *childParentNode = static_cast<TaxonomyNode *>(child.internalPointer());
  assert (childParentNode);
  
  // We avoid setting the Taxonomy descriptor as parent of an index
  if (childParentNode->getName() == "FEM")
    return QModelIndex();
  
  TaxonomyNode *grandParentNode = m_tax->getParent(childParentNode->getName());
  assert(grandParentNode);
  int row = grandParentNode->getSubElements().indexOf(childParentNode);
  // Returns the row and column of parentNode in the parentNode's parent
  return createIndex(row,0,grandParentNode);
}


//! Returned index is compossed by the row, column and parent element).
//! Thus, we don't need to store leaf nodes (which in our case can be
//! either a taxonomy or a segmentation)
QModelIndex SegmentationModel::index(int row, int column, const QModelIndex& parent) const
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
    parentNode = indexNode(parent);
  }
  assert(parentNode);
  
  // Both taxonomy and segmentation parents are Taxonomy nodes
  return createIndex(row,column,parentNode);
}

QVariant SegmentationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    // return "Segmented Objects";
   return QVariant();
}

Qt::ItemFlags SegmentationModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);
}

TaxonomyNode* SegmentationModel::indexNode(const QModelIndex& index) const
{
  TaxonomyNode *parentNode  = static_cast<TaxonomyNode *>(index.internalPointer());
  if (isLeaf(parentNode))
    return NULL; // This is a segmentation 
  else
    return parentNode->getSubElements()[index.row()];
}


bool SegmentationModel::isLeaf(TaxonomyNode* node) const
{
  return node->getSubElements().size() == 0;
}


SegmentationModel::SegmentationModel()
: m_tax(NULL)
, m_om(NULL)
{

}

SegmentationModel::~SegmentationModel()
{

}

