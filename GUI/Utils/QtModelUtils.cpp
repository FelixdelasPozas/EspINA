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

// ESPINA
#include "QtModelUtils.h"

// Qt
#include <QMetaObject>
#include <QMetaProperty>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QDebug>

//------------------------------------------------------------------------
QModelIndex QtModelUtils::findChildIndex(QModelIndex parent, QVariant value, int role)
{
  auto model = parent.model();

  for (int r = 0; model && r < model->rowCount(parent); ++r)
  {
    auto child = parent.child(r, 0);

    if (child.data(role) == value)
    {
      return child;
    }
  }

  return QModelIndex();
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

//------------------------------------------------------------------------
void QtModelUtils::dumpQObjectProperties(QObject* obj)
{
  auto mo = obj->metaObject();
  qDebug() << "## Properties of" << obj << "######";
  do
  {
    qDebug() << "--- Class" << mo->className() << "---";
    QMap<QString, QVariant> v;
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i)
    {
      v.insert(mo->property(i).name(), mo->property(i).read(obj));
    }
    auto keys = v.keys();
    qSort(keys);
    for (auto &key : keys)
    {
      qDebug() << key << "=>" << v[key];
    }
  }
  while ((mo = mo->superClass()));
  qDebug() << "###############################################";
}
