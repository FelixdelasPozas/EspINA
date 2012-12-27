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

using namespace EspINA;

//------------------------------------------------------------------------
RemoveSegmentation::SegInfo::SegInfo(SegmentationSPtr seg)
: filter(seg->filter())
, relations(seg->relations())
, segmentation(seg)
{
}

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(SegmentationPtr seg,
                                       EspinaModelSPtr  model,
                                       QUndoCommand *parent)
: QUndoCommand(parent)
, m_model(model)
{
  m_segmentations << SegInfo(m_model->findSegmentation(seg));
}

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(SegmentationList segs,
                                       EspinaModelSPtr   model,
                                       QUndoCommand    *parent)
: QUndoCommand(parent)
, m_model(model)
{
  foreach(SegmentationPtr seg, segs)
    m_segmentations << SegInfo(m_model->findSegmentation(seg));
}


//------------------------------------------------------------------------
void RemoveSegmentation::redo()
{
  SharedSegmentationList segsToRemove;
  FilterSPtrList       filtersToRemove;

  foreach(SegInfo segInfo, m_segmentations)
  {
    removeRelations(segInfo.relations);
    segsToRemove << segInfo.segmentation;
    filtersToRemove << removeFilterDependencies(segInfo.filter);
  }

  m_model->removeSegmentation(segsToRemove);

  foreach(FilterSPtr filter, filtersToRemove)
    m_model->removeFilter(filter);
}


//------------------------------------------------------------------------
void RemoveSegmentation::undo()
{
  //BUG: If several segmentations are deleted in a row, common relationships
  //     are duplicated
  foreach(FilterInfo filterInfo, m_removedFilters)
    m_model->addFilter(filterInfo.filter);

  foreach(FilterInfo filterInfo, m_removedFilters)
    addRelations(filterInfo.relations);

  m_removedFilters.clear();

  SharedSegmentationList segsToAdd;
  foreach(SegInfo segInfo, m_segmentations)
    segsToAdd << segInfo.segmentation;
  m_model->addSegmentation(segsToAdd);

  foreach(SegInfo segInfo, m_segmentations)
    addRelations(segInfo.relations);
}

//------------------------------------------------------------------------
void RemoveSegmentation::addRelations(RelationList list)
{
  foreach(Relation rel, list)
    m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);
}


//------------------------------------------------------------------------
void RemoveSegmentation::removeRelations(RelationList list)
{
  foreach(Relation rel, list)
    m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
}

//------------------------------------------------------------------------
FilterSPtrList RemoveSegmentation::removeFilterDependencies(FilterSPtr filter)
{
  FilterSPtrList filtersToRemove;

  //qDebug() << "Analyzing Filter" << filter->data().toString();
  SharedModelItemList consumers = filter->relatedItems(EspINA::OUT);
  if (consumers.isEmpty())
  {
    //qDebug() << "* Can be removed";
    filtersToRemove.push_front(filter);

    SharedModelItemList ancestors = filter->relatedItems(EspINA::IN);

    FilterInfo filterInfo(filter, filter->relations());
    m_removedFilters.push_front(filterInfo);
    removeRelations(filterInfo.relations);

    foreach(SharedModelItemPtr item, ancestors)
    {
      if (EspINA::FILTER == item->type())
        filtersToRemove << removeFilterDependencies(filterPtr(item));
      else
        if (item->type() == EspINA::CHANNEL) // 2012-12-24 Bug in pencil erase command?
          continue;
        else
          Q_ASSERT(false);
    }
  }

  return filtersToRemove;
}
