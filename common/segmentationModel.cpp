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

  TaxonomyNode *item = static_cast<TaxonomyNode *>(index.internalPointer());
  switch (role)
  {
    case Qt::DisplayRole:
      return item->getName();
    case Qt::DecorationRole:
      return QColor(255,0,0);//TODO; Change for QPixmap 
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
  
  TaxonomyNode *parentItem;
  if (!parent.isValid())
    parentItem = m_tax;
  else
    parentItem = static_cast<TaxonomyNode *>(parent.internalPointer());
  assert(parentItem);
  
  // Taxonomy leaf nodes can hold segmentations
  int childs = parentItem->getSubElements().size();
  bool isLeaf = childs == 0;
  
  if (!isLeaf)
    return parentItem->getSubElements().size();
  else
  {
    // Look for segmentations belonging to this  taxonomical class
    return m_om->segmentations(parentItem->getName()).size();
  }
}

QModelIndex SegmentationModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    return QModelIndex();
  
  TaxonomyNode *item = static_cast<TaxonomyNode *>(child.internalPointer());
  assert (item);
  
  TaxonomyNode *parentItem = m_tax->getParent(item->getName());
  assert(parentItem);
  
  // We avoid seting the Taxonomy descriptor as parent of an index
  if (parentItem->getName() == "FEM")
    return QModelIndex();
  
  TaxonomyNode *grandParent = m_tax->getParent(parentItem->getName());
  int r = grandParent->getSubElements().indexOf(parentItem);
  // Returns the row and column of parentItem in its own parent
  return createIndex(r,0,parentItem);
}

QModelIndex SegmentationModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row,column,parent))
    return QModelIndex();
  
  TaxonomyNode *parentItem;
  if (!parent.isValid())
  {// It corresponds to a taxonomy
    parentItem = m_tax;
  }
  else
  {
    parentItem = static_cast<TaxonomyNode *>(parent.internalPointer());
  }
  assert(parentItem);
  
  bool isLeaf = parentItem->getSubElements().size() == 0;
  if (isLeaf)
    // Create segmentation index
    return createIndex(row,column,m_om->segmentations(parentItem->getName())[row]);
  else
    // Create taxonomy index
    return createIndex(row,column,parentItem->getSubElements()[row]);
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

SegmentationModel::SegmentationModel()
: m_tax(NULL)
, m_om(NULL)
{

}

SegmentationModel::~SegmentationModel()
{

}

