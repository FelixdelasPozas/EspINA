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


#ifndef REMOVESEGMENTATION_H
#define REMOVESEGMENTATION_H

#include <QUndoCommand>

#include "common/model/ModelItem.h"

class Filter;
class Segmentation;

class RemoveSegmentation
: public QUndoCommand
{
  struct FilterInfo
  {
    FilterInfo () : filter(NULL) {}
    FilterInfo(Filter *filter, ModelItem::RelationList list) :
    filter(filter), relations(list)
    {}
    Filter *filter;
    ModelItem::RelationList relations;
  };
  struct SegInfo
  {
    Filter * filter;
    ModelItem::RelationList relations;
    Segmentation *segmentation;
  };
public:
  explicit RemoveSegmentation(QList<Segmentation *> segs,
			      QUndoCommand *parent=0);

  virtual void redo();
  virtual void undo();

private:
  void addRelations(ModelItem::RelationList list);
  void removeRelations(ModelItem::RelationList list);

private:
  QList<FilterInfo> m_removedFilters;
  QList<SegInfo>    m_segmentations;
};

#endif // REMOVESEGMENTATION_H
