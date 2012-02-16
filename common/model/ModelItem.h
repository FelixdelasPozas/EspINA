/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef MODELITEM_H
#define MODELITEM_H

#include <QModelIndex>

#include <QSharedPointer>

class VertexProperty;
class RelationshipGraph;

QMap<QString, QString> arguments(const QString args);

inline QString argument(const QString name, const QString value)
{
  return QString("%1=%2;").arg(name).arg(value);
}

/// Base class for every item in EspinaModel
class ModelItem
{
public:
  class Arguments : public QMap<QString, QString>
  {
  public:
    explicit Arguments();
    explicit Arguments(const QMap<QString, QString>& args);
    explicit Arguments(const QString args);
  };

  typedef QList<ModelItem *> Vector;
  enum ItemType
  { TAXONOMY
  , SAMPLE
  , CHANNEL
  , SEGMENTATION
  , FILTER};

  enum RelationType
  { IN
  , OUT
  , INOUT
  };

  ModelItem() : m_vertex(0), m_relations(NULL) {}
  virtual ~ModelItem(){}

  Vector relatedItems(RelationType rel, const QString filter = "");
  virtual QString serialize() const {return QString("none");}
  virtual QVariant data(int role) const = 0;
  virtual ItemType type() const = 0;

protected:
  size_t             m_vertex;
  RelationshipGraph *m_relations;

  friend class RelationshipGraph;
};

typedef QSharedPointer<ModelItem> ModelItemPtr;

ModelItem *indexPtr(const QModelIndex &index);

#endif //MODELITEM_H
