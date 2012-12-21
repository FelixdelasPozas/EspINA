/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "SplitUndoCommand.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Sample.h>
#include <Filters/SplitFilter.h>

using namespace EspINA;

//----------------------------------------------------------------------------
SplitUndoCommand::SplitUndoCommand(SegmentationSPtr input,
                                   FilterSPtr       filter,
                                   SegmentationSPtr splitSeg[2],
                                   EspinaModelSPtr  model)
: m_model(model)
, m_channel(input->channel())
, m_sample(m_channel->sample())
, m_seg(input)
, m_filter(filter)
, m_relations(input->relations())
{
  m_subSeg[0] = splitSeg[0];
  m_subSeg[1] = splitSeg[1];
}

//----------------------------------------------------------------------------
SplitUndoCommand::~SplitUndoCommand()
{
//   for (int i = 0; i < 2;  i++) smart pointers
//     delete m_subSeg[i];
}

//----------------------------------------------------------------------------
void SplitUndoCommand::redo()
{
  // Add new filter
  m_model->addFilter(m_filter);

  m_model->addRelation(m_seg->filter(), m_filter, SplitFilter::INPUTLINK);
  // Remove Segmentation Relations
  foreach(Relation rel, m_relations)
  {
    m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
  }

  // Remove old segmentation nodes
  m_model->removeSegmentation(m_seg);

  // Add new segmentations
  for (int i = 0; i < 2;  i++)
  {
    m_model->addSegmentation(m_subSeg[i]);

    m_model->addRelation(m_filter,  m_subSeg[i], Filter::CREATELINK);
    m_model->addRelation(m_sample,  m_subSeg[i], Sample::WHERE);
    m_model->addRelation(m_channel, m_subSeg[i], Channel::LINK);

    m_subSeg[i]->initializeExtensions();
  }
}

//----------------------------------------------------------------------------
void SplitUndoCommand::undo()
{
  // Remove segmentations
  for (int i = 0; i < 2;  i++)
  {
    foreach(Relation relation,  m_subSeg[i]->relations())
    {
      m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
    }

    m_model->removeSegmentation(m_subSeg[i]);
  }

  // Remove filter
  foreach(Relation relation,  m_filter->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeFilter(m_filter);

  // Restore input segmentation
  m_model->addSegmentation(m_seg);
  foreach(Relation rel, m_relations)
  {
    m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);
  }
}
