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

// EspINA
#include "RemoveSegmentation.h"
#include <Core/Model/Segmentation.h>
#include <Filters/ChannelReader.h>
#include <GUI/ViewManager.h>

// Qt
#include <QStack>

using namespace EspINA;

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(SegmentationPtr seg,
                                       EspinaModel  *model,
                                       ViewManager  *vm,
                                       QUndoCommand *parent)
: QUndoCommand(parent)
, m_model(model)
, m_viewManager(vm)
{
  // breadth search for related and dependent items of this segmentation in the
  // related items tree.
  QStack<SegmentationPtr> itemsStack;
  SegmentationList addedSegs;
  FilterSPtrList addedFilters;

  itemsStack.push_front(seg);
  addedSegs << seg;
  addedFilters << seg->filter();

  while (!itemsStack.isEmpty())
  {
    SegmentationPtr segmentation = itemsStack.pop();

    m_segmentations << m_model->findSegmentation(segmentation);

    foreach(Relation relation, segmentation->relations())
      if (!isADupicatedRelation(relation))
        m_relations << relation;

    foreach(ModelItemSPtr item, segmentation->relatedItems(EspINA::OUT))
      if (item->type() == SEGMENTATION)
      {
        SegmentationPtr relatedSeg = segmentationPtr(item.data());
        if (relatedSeg->isInputSegmentationDependent() && !addedSegs.contains(relatedSeg))
        {
          itemsStack.push_front(relatedSeg);
          addedSegs << relatedSeg;
          if (!addedFilters.contains(relatedSeg->filter()))
            addedFilters << relatedSeg->filter();
        }
      }
  }

  // check if the filters of the segmentations marked to remove can be also removed
  // (that is, the filters doesn't produce another segmentation not marked for removal)
  foreach(FilterSPtr filter, addedFilters)
    addFilterDependencies(filter);
}

//------------------------------------------------------------------------
RemoveSegmentation::RemoveSegmentation(SegmentationList segList,
                                       EspinaModel     *model,
                                       ViewManager     *vm,
                                       QUndoCommand    *parent)
: QUndoCommand(parent)
, m_model(model)
, m_viewManager(vm)
{
  SegmentationList addedSegs;
  FilterSPtrList addedFilters;

  foreach(SegmentationPtr seg, segList)
  {
    if (addedSegs.contains(seg))
      continue;

    // breadth search for related and dependent items of this segmentation in the
    // related items tree.
    QStack<SegmentationPtr> itemsStack;
    itemsStack.push_front(seg);
    addedSegs << seg;

    if (!addedFilters.contains(seg->filter()))
      addedFilters << seg->filter();

    while (!itemsStack.isEmpty())
    {
      SegmentationPtr segmentation = itemsStack.pop();

      m_segmentations << m_model->findSegmentation(segmentation);

      foreach(Relation relation, segmentation->relations())
        if (!isADupicatedRelation(relation))
          m_relations << relation;

      // process next dependent segmentation
      foreach(ModelItemSPtr item, segmentation->relatedItems(EspINA::OUT))
        if (item->type() == SEGMENTATION)
        {
          SegmentationPtr relatedSeg = segmentationPtr(item.data());
          if (relatedSeg->isInputSegmentationDependent() && !addedSegs.contains(relatedSeg))
          {
            itemsStack.push_front(relatedSeg);
            addedSegs << relatedSeg;
            if (!addedFilters.contains(relatedSeg->filter()))
              addedFilters << relatedSeg->filter();
          }
        }
    }
  }

  // check if the filters of the segmentations marked to remove can be also removed
  // (that is, the filters doesn't produce another segmentation not marked for removal)
  foreach(FilterSPtr filter, addedFilters)
    addFilterDependencies(filter);
}


//------------------------------------------------------------------------
void RemoveSegmentation::redo()
{
  foreach(Relation rel, m_relations)
    m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);

  m_model->removeSegmentation(m_segmentations);

  foreach(FilterSPtr filter, m_filters)
    m_model->removeFilter(filter);

  m_viewManager->updateSegmentationRepresentations();
}


//------------------------------------------------------------------------
void RemoveSegmentation::undo()
{
  foreach(FilterSPtr filter, m_filters)
    m_model->addFilter(filter);

  m_model->addSegmentation(m_segmentations);

  foreach(Relation rel, m_relations)
    m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);

  SegmentationList updatedSegs;
  foreach (SegmentationSPtr seg, m_segmentations)
    updatedSegs << seg.data();

  m_viewManager->updateSegmentationRepresentations(updatedSegs);
}

//------------------------------------------------------------------------
void RemoveSegmentation::addFilterDependencies(FilterSPtr filter)
{
  ModelItemSList consumers = filter->relatedItems(EspINA::OUT);
  foreach(ModelItemSPtr consumer, consumers)
    switch(consumer->type())
    {
      case EspINA::SEGMENTATION:
        if (!m_segmentations.contains(segmentationPtr(consumer)))
          return;
        break;
      case EspINA::FILTER:
        if (!m_filters.contains(filterPtr(consumer)))
          return;
        break;
      default:
        return;
        break;
    }

  if (!m_filters.contains(filter))
    m_filters << filter;

  foreach(Relation relation, filter->relations())
    if (!isADupicatedRelation(relation))
      m_relations << relation;

  ModelItemSList ancestors = filter->relatedItems(EspINA::IN);
  foreach(ModelItemSPtr ancestor, ancestors)
    if (ancestor->type() == EspINA::FILTER && (filterPtr(ancestor)->filterType() != ChannelReader::TYPE))
      addFilterDependencies(filterPtr(ancestor));
}

//------------------------------------------------------------------------
bool RemoveSegmentation::isADupicatedRelation(Relation relation)
{
  foreach(Relation storedRelation, m_relations)
    if (relation.ancestor == storedRelation.ancestor &&
        relation.relation == storedRelation.relation &&
        relation.succesor == storedRelation.succesor)
    {
      return true;
    }

  return false;
}
