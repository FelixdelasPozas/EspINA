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


#include "ImageLogicCommand.h"

#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Sample.h>

#include <QApplication>

const QString INPUTLINK     = "Input";
const QString MERGELINK     = "Merge";
const QString SUBSTRACTLINK = "Substract";

using namespace EspINA;

const Filter::FilterType ImageLogicCommand::FILTER_TYPE = "EditorToolBar::ImageLogicFilter";

//----------------------------------------------------------------------------
ImageLogicCommand::ImageLogicCommand(SegmentationList            input,
                                     ImageLogicFilter::Operation operation,
                                     TaxonomyElementPtr          taxonomy,
                                     EspinaModel                *model,
                                     QUndoCommand *              parent)
: QUndoCommand(parent)
, m_model(model)
, m_operation(operation)
, m_taxonomy(m_model->findTaxonomyElement(taxonomy))
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Filter::NamedInputs inputs;
  Filter::Arguments args;
  ImageLogicFilter::Parameters params(args);
  for(int i = 0; i < input.size(); i++)
  {
    SegmentationSPtr seg = m_model->findSegmentation(input[i]);
    m_input << seg;

    if (i > 0)
      args[Filter::INPUTS].append(",");

    args[Filter::INPUTS].append(Filter::NamedInput(link(seg), seg->outputId()));

    inputs[link(seg)] = seg->filter();

    m_infoList << SegInfo(seg);
  }
  params.setOperation(m_operation);

  m_filter = FilterSPtr(new ImageLogicFilter(inputs, args, ImageLogicCommand::FILTER_TYPE));
  m_filter->update();
  Q_ASSERT(m_filter->outputs().size() == 1);
  m_segmentation = m_model->factory()->createSegmentation(m_filter, 0);

  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
const QString ImageLogicCommand::link(SegmentationSPtr seg)
{
  unsigned int index = m_input.indexOf(seg);

  QString linkName;
  if (0 == index)
    linkName = INPUTLINK;

  else if (ImageLogicFilter::ADDITION == m_operation)
    linkName = MERGELINK + QString::number(index);

  else if (ImageLogicFilter::SUBSTRACTION == m_operation)
    linkName = SUBSTRACTLINK + QString::number(index);

  else
    Q_ASSERT(false);

  return linkName;
}


//----------------------------------------------------------------------------
void ImageLogicCommand::redo()
{
  //TODO: Combine segmentations from different channels
  ChannelSPtr channel = m_input.first()->channel();

  // Add new filter
  m_model->addFilter(m_filter);
  SegmentationSList oldSegmentations;
  foreach(SegInfo info, m_infoList)
  {
    m_model->addRelation(info.filter, m_filter, link(info.segmentation));
    // Remove Segmentation Relations
    foreach(EspINA::Relation rel, info.relations)
    {
      m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
    }
    oldSegmentations << info.segmentation;
  }
  // Remove old segmentation nodes
  m_model->removeSegmentation(oldSegmentations);

  // Add new segmentation
  m_segmentation->setTaxonomy(m_taxonomy);
  m_model->addSegmentation(m_segmentation);

  SampleSPtr sample = channel->sample();
  m_model->addRelation(m_filter, m_segmentation, Filter::CREATELINK);
  m_model->addRelation(sample  , m_segmentation, Sample::WHERE);
  m_model->addRelation(channel , m_segmentation, Channel::LINK);
}

//----------------------------------------------------------------------------
void ImageLogicCommand::undo()
{
  // Remove merge segmentation
  foreach(EspINA::Relation relation,  m_segmentation->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeSegmentation(m_segmentation);

  // Remove filter
  foreach(EspINA::Relation relation,  m_filter->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeFilter(m_filter);

  // Restore input segmentation
  // First we need to restore all segmentations, just in case there are
  // relations between segmentations
  m_model->addSegmentation(m_input);
  foreach(SegInfo info, m_infoList)
  {
    foreach(EspINA::Relation rel, info.relations)
    {
      m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);
    }
  }
}