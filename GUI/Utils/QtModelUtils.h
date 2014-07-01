/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
  QModelIndex findChildIndex(QModelIndex parent, QVariant value, int role = Qt::DisplayRole);

  /// Return a list of all QModelIndex between @topLeft index and @bottomRight index
  QModelIndexList indices(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  /// Return whether a node has leaf nodes
  bool isInnerNode(const QModelIndex &index);

  /// Return whether a node is leaf
  bool isLeafNode(const QModelIndex &index);
} // namespace QtModelUtils

#endif // QTMODELUTILS_H
