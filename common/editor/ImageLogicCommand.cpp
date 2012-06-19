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

ImageLogicCommand::ImageLogicCommand(QList<Segmentation *> inputs,
				     ImageLogicFilter::Operation op)
: m_input(inputs)
{
  m_filter = new ImageLogicFilter(inputs, op);
}

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
    model->addRelation(seg, m_filter, "Add");
  }
  Segmentation *seg = m_filter->product(0);
  seg->setTaxonomy(EspinaCore::instance()->activeTaxonomy());
  model->addSegmentation(seg);
  foreach(Channel *channel, channels)
  {
    Sample *sample = channel->sample();
    model->addRelation(m_filter, seg, "CreateSegmentation");
    model->addRelation(sample, seg, "where");
    model->addRelation(channel, seg, "Channel");
    seg->initialize();
  }
}

void ImageLogicCommand::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  Segmentation *seg = m_filter->product(0);

  foreach(ModelItem::Relation relation,  seg->relations())
  {
    model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  model->removeSegmentation(seg);

  foreach(ModelItem::Relation relation,  m_filter->relations())
  {
    model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
  }
  model->removeFilter(m_filter);
}
