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


#include "AddSegmentation.h"

#include "common/EspinaCore.h"
#include <model/Channel.h>
#include <model/Segmentation.h>

//----------------------------------------------------------------------------
AddSegmentation::AddSegmentation(Channel      *channel,
                                 Filter       *filter,
                                 Segmentation *seg,
                                 TaxonomyNode *taxonomy)
: m_channel (channel)
, m_filter  (filter)
, m_seg     (seg)
, m_taxonomy(taxonomy)
{
  ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN, Channel::STAINLINK);
  Q_ASSERT(samples.size() > 0);
  m_sample = dynamic_cast<Sample *>(samples.first());
}

//----------------------------------------------------------------------------
void AddSegmentation::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->addFilter(m_filter);
  model->addRelation(m_channel, m_filter, "Channel");
  m_seg->setTaxonomy(m_taxonomy);
  model->addSegmentation(m_seg);
  model->addRelation(m_filter, m_seg, CREATELINK);
  model->addRelation(m_sample, m_seg, "where");
  model->addRelation(m_channel, m_seg, "Channel");
  m_seg->initialize();
}

//----------------------------------------------------------------------------
void AddSegmentation::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->removeRelation(m_channel, m_seg, "Channel");
  model->removeRelation(m_sample, m_seg, "where");
  model->removeRelation(m_filter, m_seg, CREATELINK);
  model->removeSegmentation(m_seg);
  model->removeRelation(m_channel, m_filter, "Channel");
  model->removeFilter(m_filter);
}