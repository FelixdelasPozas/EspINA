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

#include <EspinaCore.h>

#include <common/model/Channel.h>
#include <model/EspinaFactory.h>

const QString INPUTLINK     = "Input";
const QString MERGELINK     = "Merge";
const QString SUBSTRACTLINK = "Substract";

//----------------------------------------------------------------------------
ImageLogicCommand::ImageLogicCommand(QList<Segmentation *> segmentations,
                                     ImageLogicFilter::Operation op)
: m_input(segmentations)
, m_op(op)
{
  Filter::NamedInputs inputs;
  Filter::Arguments args;
  ImageLogicFilter::Parameters params(args);
  Segmentation *seg = segmentations.first();
  args[Filter::INPUTS] = link(seg) + "_" + QString::number(seg->outputNumber());
  inputs[link(seg)] = seg->filter();
  for(int i=1; i<segmentations.size(); i++)
  {
    seg = segmentations[i];
    args[Filter::INPUTS].append(",");
    args[Filter::INPUTS].append(link(seg) + "_" + QString::number(seg->outputNumber()));
    inputs[link(seg)] = seg->filter();
  }
  params.setOperation(op);
  m_filter = new ImageLogicFilter(inputs, args);
  m_filter->update();
  m_seg = EspinaFactory::instance()->createSegmentation(m_filter, 0);
  m_tax = EspinaCore::instance()->activeTaxonomy();
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
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  QSet<Channel *> channels;
  foreach(Segmentation *seg, m_input)
  {
    ModelItem::Vector relatedChannels = seg->relatedItems(ModelItem::IN, "Channel");
    foreach(ModelItem *item, relatedChannels)
    {
      Channel *channel = dynamic_cast<Channel *>(item);
      //model->removeRelation(channel, seg, "Channel");
      channels << channel;
    }
  }
  model->addFilter(m_filter);
  foreach(Segmentation *seg, m_input)
  {
    model->addRelation(seg, m_filter, link(seg));
  }
  m_seg->setTaxonomy(m_tax);
  model->addSegmentation(m_seg);
  foreach(Channel *channel, channels)
  {
    Sample *sample = channel->sample();
    model->addRelation(m_filter, m_seg, CREATELINK);
    model->addRelation(sample, m_seg, "where");
    model->addRelation(channel, m_seg, "Channel");
    m_seg->initialize();
  }
}

//----------------------------------------------------------------------------
void ImageLogicCommand::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  foreach(ModelItem::Relation relation,  m_seg->relations())
  {
    model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  model->removeSegmentation(m_seg);

  foreach(ModelItem::Relation relation,  m_filter->relations())
  {
    model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  model->removeFilter(m_filter);
}
