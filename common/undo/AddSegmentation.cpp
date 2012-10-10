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

#include "common/model/Channel.h"
#include "common/model/EspinaModel.h"
#include "common/model/Sample.h"
#include "common/model/Segmentation.h"

//----------------------------------------------------------------------------
AddSegmentation::AddSegmentation(Channel         *channel,
                                 Filter          *filter,
                                 Segmentation    *seg,
                                 TaxonomyElement *taxonomy,
                                 EspinaModel     *model)
: m_channel (channel)
, m_filter  (filter)
, m_seg     (seg)
, m_taxonomy(taxonomy)
, m_model   (model)
{
  //TODO: Poner punto de interrupcion y comprobar si SIEMPRE se crean segmentacion antes de invocar a este m�todo
  //      en cuyo caso hay que destruirlas en el destructor
  m_sample = m_channel->sample();
  Q_ASSERT(m_sample);
}

//----------------------------------------------------------------------------
void AddSegmentation::redo()
{
  m_model->addFilter(m_filter);
  m_model->addRelation(m_channel, m_filter, Channel::LINK);
  m_seg->setTaxonomy(m_taxonomy);
  m_model->addSegmentation(m_seg);
  m_model->addRelation(m_filter, m_seg, CREATELINK);
  m_model->addRelation(m_sample, m_seg, Sample::WHERE);
  m_model->addRelation(m_channel, m_seg, Channel::LINK);
  m_seg->initializeExtensions();
}

//----------------------------------------------------------------------------
void AddSegmentation::undo()
{
  m_model->removeRelation(m_channel, m_seg, Channel::LINK);
  m_model->removeRelation(m_sample, m_seg, "where");
  m_model->removeRelation(m_filter, m_seg, CREATELINK);
  m_model->removeSegmentation(m_seg);
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  m_model->removeFilter(m_filter);
}