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

#include "common/model/EspinaModel.h"
#include "common/model/Channel.h"
#include <model/ChannelReader.h>
#include "common/EspinaCore.h"
#include <selection/SelectionManager.h>

AddChannel::AddChannel(ChannelReader *reader,
                       Channel* channel,
                       QUndoCommand* parent)
: QUndoCommand(parent)
, m_reader(reader)
, m_channel(channel)
{
}

void AddChannel::redo()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  model->addFilter(m_reader);
  model->addChannel(m_channel);
  SelectionManager::instance()->setActiveChannel(m_channel);
  model->addRelation(m_reader, m_channel, Channel::VOLUMELINK);
}

void AddChannel::undo()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  model->removeRelation(m_reader, m_channel, Channel::VOLUMELINK);
  model->removeChannel(m_channel);
  model->removeFilter(m_reader);
}

