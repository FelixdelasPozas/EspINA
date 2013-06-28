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


#ifndef QTMODELUTILS_H
#define QTMODELUTILS_H

#include "EspinaCore_Export.h"

#include <QModelIndex>

namespace QtModelUtils
{
  /// Find the QModelIndex children of @parent whose @role is equal to @value
  QModelIndex EspinaCore_EXPORT findChildIndex(QModelIndex parent, QVariant value, int role = Qt::DisplayRole);

  /// Return a list of all QModelIndex between @topLeft index and @bottomRight index
  QModelIndexList EspinaCore_EXPORT indices(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  /// Return whether a node has leaf nodes
  bool EspinaCore_EXPORT isInnerNode(const QModelIndex &index);

  /// Return whether a node is leaf
  bool EspinaCore_EXPORT isLeafNode(const QModelIndex &index);
} // namespace QtModelUtils

#endif // QTMODELUTILS_H
