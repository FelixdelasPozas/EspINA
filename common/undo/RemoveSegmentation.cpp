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

#include "common/EspinaCore.h"
#include "common/model/Segmentation.h"


//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(QList<Segmentation *> segs,
				       QUndoCommand* parent)
: QUndoCommand(parent)
{
  m_segmentations.clear();
  m_removedFilters.clear();

  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
  Segmentation *seg;
  foreach(seg, segs)
  {
    SegInfo info;
    info.filter = seg->filter();
    info.relations = seg->relations();
    info.segmentation = seg;
    m_segmentations << info;
  }
}


//------------------------------------------------------------------------
void RemoveSegmentation::redo()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  QList<Segmentation *> segsToRemove;
  QList<Filter *>    filtersToRemove;

  SegInfo segInfo;
  foreach(segInfo, m_segmentations)
  {
    removeRelations(segInfo.relations);
    segsToRemove << segInfo.segmentation;
    ModelItem::Vector segs = segInfo.filter->relatedItems(ModelItem::OUT, CREATELINK);
    if (segs.size() == 0)
    {
//       qDebug() << segInfo.filter->data(Qt::DisplayRole).toString() << "has no segmentations==>Must be deleted";
      FilterInfo filterInfo(segInfo.filter, segInfo.filter->relations());
      removeRelations(filterInfo.relations);
      filtersToRemove << filterInfo.filter;
      m_removedFilters << filterInfo;
    }
  }
  model->removeSegmentation(segsToRemove);
  foreach(Filter *filter, filtersToRemove)
  {
    model->removeFilter(filter);
  }
}


//------------------------------------------------------------------------
void RemoveSegmentation::undo()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  FilterInfo filterInfo;
  foreach(filterInfo, m_removedFilters)
  {
    model->addFilter(filterInfo.filter);
    addRelations(filterInfo.relations);
  }
  m_removedFilters.clear();

  QList<Segmentation *> segsToAdd;
  SegInfo segInfo;
  foreach(segInfo, m_segmentations)
  {
    segsToAdd << segInfo.segmentation;
  }
  model->addSegmentation(segsToAdd);
  foreach(segInfo, m_segmentations)
  {
    addRelations(segInfo.relations);
  }
}

//------------------------------------------------------------------------
void RemoveSegmentation::addRelations(ModelItem::RelationList list)
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
  foreach(ModelItem::Relation rel, list)
  {
    model->addRelation(rel.ancestor, rel.succesor, rel.relation);
  }
}


//------------------------------------------------------------------------
void RemoveSegmentation::removeRelations(ModelItem::RelationList list)
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();
  foreach(ModelItem::Relation rel, list)
  {
    model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
  }
}
