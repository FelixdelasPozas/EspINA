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


#include "RemoveSegmentation.h"

#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>

//------------------------------------------------------------------------
RemoveSegmentation::SegInfo::SegInfo(Segmentation* seg)
: filter(seg->filter())
, relations(seg->relations())
, segmentation(seg)
{
}

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(Segmentation* seg,
                                       EspinaModel* model,
                                       QUndoCommand* parent)
: QUndoCommand(parent)
, m_model(model)
{
  m_segmentations << SegInfo(seg);
}

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(QList<Segmentation *> segs,
                                       EspinaModel          *model,
                                       QUndoCommand         *parent)
: QUndoCommand(parent)
, m_model(model)
{
  foreach(Segmentation *seg, segs)
    m_segmentations << SegInfo(seg);
}


//------------------------------------------------------------------------
void RemoveSegmentation::redo()
{
  QList<Segmentation *> segsToRemove;
  QList<Filter *>    filtersToRemove;

  foreach(SegInfo segInfo, m_segmentations)
  {
    removeRelations(segInfo.relations);
    segsToRemove << segInfo.segmentation;
    filtersToRemove << removeFilterDependencies(segInfo.filter);
  }

  m_model->removeSegmentation(segsToRemove);

  foreach(Filter *filter, filtersToRemove)
    m_model->removeFilter(filter);
}


//------------------------------------------------------------------------
void RemoveSegmentation::undo()
{
  foreach(FilterInfo filterInfo, m_removedFilters)
    m_model->addFilter(filterInfo.filter);

  foreach(FilterInfo filterInfo, m_removedFilters)
    addRelations(filterInfo.relations);

  m_removedFilters.clear();

  QList<Segmentation *> segsToAdd;
  foreach(SegInfo segInfo, m_segmentations)
    segsToAdd << segInfo.segmentation;
  m_model->addSegmentation(segsToAdd);

  foreach(SegInfo segInfo, m_segmentations)
    addRelations(segInfo.relations);
}

//------------------------------------------------------------------------
void RemoveSegmentation::addRelations(ModelItem::RelationList list)
{
  foreach(ModelItem::Relation rel, list)
    m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);
}


//------------------------------------------------------------------------
void RemoveSegmentation::removeRelations(ModelItem::RelationList list)
{
  foreach(ModelItem::Relation rel, list)
    m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
}

//------------------------------------------------------------------------
QList<Filter *> RemoveSegmentation::removeFilterDependencies(Filter* filter)
{
  QList<Filter *> filtersToRemove;

  //qDebug() << "Analyzing Filter" << filter->data().toString();
  ModelItem::Vector consumers = filter->relatedItems(ModelItem::OUT);
  if (consumers.isEmpty())
  {
    //qDebug() << "* Can be removed";
    filtersToRemove.push_front(filter);

    ModelItem::Vector ancestors = filter->relatedItems(ModelItem::IN);

    FilterInfo filterInfo(filter, filter->relations());
    m_removedFilters.push_front(filterInfo);
    removeRelations(filterInfo.relations);

    foreach(ModelItem *item, ancestors)
    {
      if (ModelItem::FILTER == item->type())
        filtersToRemove << removeFilterDependencies(dynamic_cast<Filter *>(item));
      else
        Q_ASSERT(false);
    }
  }

  return filtersToRemove;
}
