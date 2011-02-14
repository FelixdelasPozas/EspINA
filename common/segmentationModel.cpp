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
#include <data/taxonomy.h>

#include <QDebug>

QVariant SegmentationModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();
  
  QString texto = "Variable";
  return texto;
}

int SegmentationModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}

int SegmentationModel::rowCount(const QModelIndex& parent) const
{
  TaxonomyNode *parentItem;
  if (!parent.isValid())
    parentItem = m_tax;
  else
    parentItem = static_cast<TaxonomyNode *>(parent.internalPointer());
  return parentItem->getSubElements()->size();
  
}

QModelIndex SegmentationModel::parent(const QModelIndex& child) const
{
  TaxonomyNode *item = static_cast<TaxonomyNode *>(child.internalPointer());
  qDebug() << item->getName();
  if (!child.isValid())
    return QModelIndex();
  int r;
  for (r =0; m_tax->getSubElements()->size();r++)
    if (m_tax->getSubElements()->at(r)->getName() == item->getName())
      break;
  return createIndex(r,0,m_tax);
}

QModelIndex SegmentationModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!parent.isValid())
  {// It corresponds to a taxonomy
    TaxonomyNode *tax = m_tax->getSubElements()->at(row);
    qDebug() << tax->getName();
    return createIndex(row,column,tax);
  }
  else
    return QModelIndex();
}

QVariant SegmentationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags SegmentationModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);
}

SegmentationModel::SegmentationModel()
{

}

SegmentationModel::~SegmentationModel()
{

}

