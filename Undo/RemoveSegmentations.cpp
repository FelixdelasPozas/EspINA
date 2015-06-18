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
#include "RemoveSegmentations.h"

// Qt
#include <QStack>

using namespace ESPINA;

//------------------------------------------------------------------------
RemoveSegmentations::RemoveSegmentations(SegmentationAdapterPtr segmentation,
                                         ModelAdapterSPtr       model,
                                         QUndoCommand          *parent)
: QUndoCommand{parent}
, m_model     {model}
{
  analyzeSegmentation(segmentation);
}

//------------------------------------------------------------------------
RemoveSegmentations::RemoveSegmentations(SegmentationAdapterList segmentations,
                                         ModelAdapterSPtr        model,
                                         QUndoCommand           *parent)
: QUndoCommand{parent}
, m_model     {model}
{
  for (auto segmentation : segmentations)
  {
    analyzeSegmentation(segmentation);
  }
}

//------------------------------------------------------------------------
void RemoveSegmentations::analyzeSegmentation(SegmentationAdapterPtr segmentation)
{
  // Breadth-first search for related and dependent items of this
  // segmentation in the related items tree.
  QStack<SegmentationAdapterPtr> stack;
  SegmentationAdapterList        dependentSegmentations;
  FilterSList addedFilters;

  stack.push_front(segmentation);
  dependentSegmentations << segmentation;

  while (!stack.isEmpty())
  {
    auto segmentation = stack.pop();

    m_segmentations << m_model->smartPointer(segmentation);

    m_relations << m_model->relations(segmentation, ESPINA::RELATION_INOUT);

    //TODO: Add segmentations a new flag to indicate whether it has to be deleted if
    //      its input is deleted or add an special relation to notify that

//    for(auto item : m_model->relatedItems(segmentation, ESPINA::RELATION_OUT))
//    {
//      if (ItemAdapter::Type::SEGMENTATION == item->type())
//      {
//        auto dependentSegmentation = segmentationPtr(item.get());
//        if ( dependentSegmentation->isInputSegmentationDependent()
//             && !dependentSegmentations.contains(dependentSegmentation))
//        {
//          stack.push_front(dependentSegmentation);
//          dependentSegmentations << dependentSegmentation;
//        }
//      }
//    }
  }
}

//------------------------------------------------------------------------
void RemoveSegmentations::redo()
{
  m_model->beginBatchMode();
  for(auto relation : m_relations)
  {
    m_model->deleteRelation(relation);
  }

  m_model->remove(m_segmentations);
  m_model->endBatchMode();
}


//------------------------------------------------------------------------
void RemoveSegmentations::undo()
{
  m_model->beginBatchMode();
  m_model->add(m_segmentations);

  for(Relation relation : m_relations)
  {
    m_model->addRelation(relation);
  }
  m_model->endBatchMode();
}

// //------------------------------------------------------------------------
// void RemoveSegmentations::addFilterDependencies(FilterSPtr filter)
// {
//   ModelItemSList consumers = filter->relatedItems(ESPINA::RELATION_OUT);
//   foreach(ModelItemSPtr consumer, consumers)
//     switch(consumer->type())
//     {
//       case ESPINA::SEGMENTATION:
//         if (!m_segmentations.contains(segmentationPtr(consumer)))
//           return;
//         break;
//       case ESPINA::FILTER:
//         if (!m_filters.contains(filterPtr(consumer)))
//           return;
//         break;
//       default:
//         return;
//         break;
//     }
//
//   if (!m_filters.contains(filter))
//     m_filters << filter;
//
//   foreach(Relation relation, filter->relations())
//     if (!isADupicatedRelation(relation))
//       m_relations << relation;
//
//   ModelItemSList ancestors = filter->relatedItems(ESPINA::RELATION_IN);
//   foreach(ModelItemSPtr ancestor, ancestors)
//     if (ancestor->type() == ESPINA::FILTER && (filterPtr(ancestor)->filterType() != ChannelReader::TYPE))
//       addFilterDependencies(filterPtr(ancestor));
// }

// //------------------------------------------------------------------------
// bool RemoveSegmentations::isADupicatedRelation(Relation relation)
// {
//   foreach(Relation storedRelation, m_relations)
//     if (relation.ancestor == storedRelation.ancestor &&
//         relation.relation == storedRelation.relation &&
//         relation.succesor == storedRelation.succesor)
//     {
//       return true;
//     }
//
//   return false;
// }
