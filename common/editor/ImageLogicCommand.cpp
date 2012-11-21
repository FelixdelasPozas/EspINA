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

#include "common/model/Channel.h"
#include "common/model/Segmentation.h"
#include "common/model/EspinaFactory.h"
#include <model/EspinaModel.h>
#include <QApplication>

const QString INPUTLINK     = "Input";
const QString MERGELINK     = "Merge";
const QString SUBSTRACTLINK = "Substract";

//----------------------------------------------------------------------------
ImageLogicCommand::ImageLogicCommand(QList<Segmentation *> segmentations,
                                     ImageLogicFilter::Operation op,
                                     EspinaModel *model,
                                     TaxonomyElement *taxonomy)
: m_model(model)
, m_input(segmentations)
, m_op(op)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Filter::NamedInputs inputs;
  Filter::Arguments args;
  ImageLogicFilter::Parameters params(args);
  for(int i=0; i<segmentations.size(); i++)
  {
    Segmentation *seg = segmentations[i];
    if (i>0) args[Filter::INPUTS].append(",");
    args[Filter::INPUTS].append(Filter::NamedInput(link(seg), seg->outputId()));
    inputs[link(seg)] = seg->filter();
    m_infoList << SegInfo(seg);
  }
  params.setOperation(op);
  m_filter = new ImageLogicFilter(inputs, args);
  m_filter->update();
  m_seg = m_model->factory()->createSegmentation(m_filter, 0);
  m_tax = taxonomy;

  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
const QString ImageLogicCommand::link(Segmentation* seg)
{
  unsigned int index = m_input.indexOf(seg);
  QString linkName;
  if (0 == index)
    linkName = INPUTLINK;
  else if (ImageLogicFilter::ADDITION == m_op)
    linkName = MERGELINK + QString::number(index);
  else if (ImageLogicFilter::SUBSTRACTION == m_op)
    linkName = SUBSTRACTLINK + QString::number(index);
  else
    Q_ASSERT(false);

  return linkName;
}


//----------------------------------------------------------------------------
void ImageLogicCommand::redo()
{
  //TODO: Combine segmentations from different channels
  QSet<Channel *> channels;
  foreach(Segmentation *seg, m_input)
  {
    ModelItem::Vector relatedChannels = seg->relatedItems(ModelItem::IN, Channel::LINK);
    foreach(ModelItem *item, relatedChannels)
    {
      Channel *channel = dynamic_cast<Channel *>(item);
      //model->removeRelation(channel, seg, "Channel");
      channels << channel;
    }
  }
  // Add new filter
  m_model->addFilter(m_filter);
  QList<Segmentation *> oldSegmentations;
  foreach(SegInfo info, m_infoList)
  {
    ModelItem::Vector segFilter = info.segmentation->relatedItems(ModelItem::IN, CREATELINK);
    Q_ASSERT(segFilter.size() == 1);
    ModelItem *item = segFilter[0];
    Q_ASSERT(ModelItem::FILTER == item->type());
    m_model->addRelation(item, m_filter, link(info.segmentation));
    // Remove Segmentation Relations
    foreach(ModelItem::Relation rel, info.relations)
    {
      m_model->removeRelation(rel.ancestor, rel.succesor, rel.relation);
    }
    oldSegmentations << info.segmentation;
  }
  // Remove old segmentation nodes
  m_model->removeSegmentation(oldSegmentations);

  // Add new segmentation
  m_seg->setTaxonomy(m_tax);
  m_model->addSegmentation(m_seg);
  foreach(Channel *channel, channels)
  {
    Sample *sample = channel->sample();
    m_model->addRelation(m_filter, m_seg, CREATELINK);
    m_model->addRelation(sample,   m_seg, Sample::WHERE);
    m_model->addRelation(channel,  m_seg, Channel::LINK);
  }
  m_seg->initializeExtensions();
}

//----------------------------------------------------------------------------
void ImageLogicCommand::undo()
{
  // Remove merge segmentation
  foreach(ModelItem::Relation relation,  m_seg->relations())
  {
    m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  m_model->removeSegmentation(m_seg);

  // Remove filter
  foreach(ModelItem::Relation relation,  m_filter->relations())
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
    foreach(ModelItem::Relation rel, info.relations)
    {
      m_model->addRelation(rel.ancestor, rel.succesor, rel.relation);
    }
  }
}