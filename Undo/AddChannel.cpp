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


#include "AddChannel.h"

#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Filters/ChannelReader.h>

AddChannel::AddChannel(ChannelReader *reader,
                       Channel       *channel,
                       EspinaModel   *model,
                       QUndoCommand  *parent)
: QUndoCommand(parent)
, m_reader (reader)
, m_channel(channel)
, m_model  (model)
{
}

void AddChannel::redo()
{
  m_model->addFilter(m_reader);
  m_model->addChannel(m_channel);
  m_model->addRelation(m_reader, m_channel, Channel::VOLUMELINK);
}

void AddChannel::undo()
{
  m_model->removeRelation(m_reader, m_channel, Channel::VOLUMELINK);
  m_model->removeChannel(m_channel);
  m_model->removeFilter(m_reader);
}

