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

/// Base class for every item in EspinaModel
class ModelItem
{
public:
  enum ItemType
  { TAXONOMY
  , SAMPLE
  , CHANNEL
  , SEGMENTATION
  , FILTER};

  ModelItem() : m_vertex(NULL), m_relations(NULL) {}
  virtual ~ModelItem(){}

  virtual QVariant data(int role) const = 0;
  virtual ItemType type() const = 0;

protected:
  VertexProperty *m_vertex;
  RelationshipGraph *m_relations;

  friend class RelationshipGraph;
};

typedef QSharedPointer<ModelItem> ModelItemPtr;

ModelItem *indexPtr(const QModelIndex &index);

#endif //MODELITEM_H
