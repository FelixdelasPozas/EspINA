/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "UnloadChannelCommand.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Filter.h>

//------------------------------------------------------------------------
EspINA::UnloadChannelCommand::UnloadChannelCommand(ChannelPtr   channel,
                                                   EspinaModel  *model,
                                                   QUndoCommand *parent)
: QUndoCommand(parent)
, m_model     (model)
, m_sample    (channel->sample())
, m_reader    (channel->filter())
, m_channel   (model->findChannel(channel))
{

}

//------------------------------------------------------------------------
void EspINA::UnloadChannelCommand::redo()
{
  m_model->removeRelation(m_sample, m_channel, Channel::STAIN_LINK);
  m_model->removeRelation(m_reader, m_channel, Channel::VOLUME_LINK);
  m_model->removeChannel (m_channel);
  m_model->removeFilter  (m_reader);

  // updates EspinaRenderViews bounds and makes a render
  // FIXME: m_channel->volume()->markAsModified();
  Q_ASSERT(false);
}

//------------------------------------------------------------------------
void EspINA::UnloadChannelCommand::undo()
{
  m_model->addFilter  (m_reader);
  m_model->addChannel (m_channel);
  m_model->addRelation(m_reader, m_channel, Channel::VOLUME_LINK);
  m_model->addRelation(m_sample, m_channel, Channel::STAIN_LINK);

  // updates EspinaRenderViews bounds and makes a render
  //FIXME:m_channel->volume()->markAsModified();
  Q_ASSERT(false);
}

