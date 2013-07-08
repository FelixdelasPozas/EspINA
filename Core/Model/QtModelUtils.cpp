/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

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


#include "QtModelUtils.h"

//------------------------------------------------------------------------
QModelIndex QtModelUtils::findChildIndex(QModelIndex parent, QVariant value, int role)
{
  QModelIndex index;
  const QAbstractItemModel *model = parent.model();

  int r = 0;
  while (!index.isValid() && model && r < model->rowCount(parent))
  {
    QModelIndex child = parent.child(r, 0);
    if (child.data(role) == value)
      index = child;
    else
      index = findChildIndex(child, value, role);
    ++r;
  }

  return index;
}

//------------------------------------------------------------------------
QModelIndexList QtModelUtils::indices(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  QModelIndexList result;

  result << topLeft;

  const QAbstractItemModel *model = topLeft.model();
  if (topLeft != bottomRight)
  {
    for (int r = 0; r < model->rowCount(topLeft); r++)
    {
      result << indices(topLeft.child(r, 0), bottomRight);
    }

    for (int r = topLeft.row(); r < model->rowCount(topLeft.parent()); r++)
      result << indices(topLeft.sibling(r,0), bottomRight);
  }

  return result;
}

//------------------------------------------------------------------------
bool QtModelUtils::isInnerNode(const QModelIndex &index)
{
  bool hasLeafNode = false;

  if (index.isValid())
  {
    const QAbstractItemModel *model = index.model();

    int i = 0;
    while (!hasLeafNode && i < model->rowCount(index))
    {
      hasLeafNode |= isLeafNode(index.child(i, 0));
      ++i;
    }
  }

  return hasLeafNode;
}

//------------------------------------------------------------------------
bool QtModelUtils::isLeafNode(const QModelIndex &index)
{
  return index.isValid() && (index.model()->rowCount(index) == 0);
}