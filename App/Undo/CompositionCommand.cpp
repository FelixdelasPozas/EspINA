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


#include "CompositionCommand.h"

#include <Core/Model/Channel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>

#include <QApplication>

using namespace EspINA;

const QString INPUTLINK     = "Input";
const QString MERGELINK     = "Merge";
const QString SUBSTRACTLINK = "Substract";

const Filter::FilterType CompositionCommand::FILTER_TYPE = "CompositionToolbar::CompositionFilter";

//----------------------------------------------------------------------------
CompositionCommand::CompositionCommand(const SegmentationList &segmentations,
                                       TaxonomyElementSPtr      taxonomy,
                                       EspinaModel          *model)
: m_model(model)
, m_tax(taxonomy)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Filter::NamedInputs inputs;
  Filter::Arguments args;
  ImageLogicFilter::Parameters params(args);
  for(int i=0; i<segmentations.size(); i++)
  {
    SegmentationSPtr seg = m_model->findSegmentation(segmentations[i]);
    if (i>0) args[Filter::INPUTS].append(",");
    args[Filter::INPUTS].append(Filter::NamedInput(link(seg), seg->outputId()));
    inputs[link(seg)] = seg->filter();
    m_input    << seg;
    m_infoList << SegInfo(seg);
  }
  params.setOperation(ImageLogicFilter::ADDITION);
  m_filter = FilterSPtr(new ImageLogicFilter(inputs, args, FILTER_TYPE));
  m_filter->update();
  m_seg = m_model->factory()->createSegmentation(m_filter, 0);

  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
const QString CompositionCommand::link(SegmentationSPtr seg)
{
  unsigned int index = m_input.indexOf(seg);
  QString linkName;
  if (0 == index)
    linkName = INPUTLINK;
  else
    linkName = MERGELINK + QString::number(index);

  return linkName;
}


//----------------------------------------------------------------------------
void CompositionCommand::redo()
{
  // Add new filter
  m_model->addFilter(m_filter);
  foreach(SegmentationSPtr seg, m_input)
  {
//     SharedModelItemList segFilter = seg->relatedItems(EspINA::IN, CREATELINK);
//     Q_ASSERT(segFilter.size() == 1);
//     SharedModelItemPtr item = segFilter[0];
//     Q_ASSERT(EspINA::FILTER == item->type());
    m_model->addRelation(seg->filter(), m_filter, link(seg));
  }

  // Add new segmentation
  m_seg->setTaxonomy(m_tax);
  m_model->addSegmentation(m_seg);

  //WARNING: This won't work with segmentation belonging to different channels
  ChannelSPtr channel = m_input.first()->channel();
  SampleSPtr  sample  = channel->sample();
  m_model->addRelation(m_filter, m_seg, Filter::CREATELINK);
  m_model->addRelation(sample,   m_seg, Sample::WHERE);
  m_model->addRelation(channel,  m_seg, Channel::LINK);

  foreach(SegmentationSPtr part, m_input)
  {
    m_model->addRelation(m_seg, part, Segmentation::COMPOSED_LINK);
  }

  m_seg->initializeExtensions();
}

//----------------------------------------------------------------------------
void CompositionCommand::undo()
{
  // Remove merge segmentation
  foreach(Relation relation,  m_seg->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeSegmentation(m_seg);

  // Remove filter
  foreach(Relation relation,  m_filter->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeFilter(m_filter);
}