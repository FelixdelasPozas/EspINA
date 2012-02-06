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

#include <common/cache/CachedObjectBuilder.h>
#include <common/model/EspINA.h>
#include <common/model/Channel.h>
#include <common/model/Sample.h>

#include <pqPipelineSource.h>
#include <pqLoadDataReaction.h>


AddChannel::AddChannel(QSharedPointer< EspINA > model,
		       QSharedPointer< Sample > sample,
		       const QString channelFile,
		       QUndoCommand* parent)
: QUndoCommand(parent)
, m_model(model)
, m_sample(sample)
, m_file(channelFile)
{
}

void AddChannel::redo()
{
  pqPipelineSource* reader = pqLoadDataReaction::loadData(QStringList(m_file));
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_reader = cob->registerFilter(m_file, reader);
  Q_ASSERT(m_reader->getNumberOfData() == 1);
  pqData channelData(m_reader,0);
  m_channel = new Channel(channelData);
  m_model->addChannel(m_sample.data(), m_channel);
}

void AddChannel::undo()
{
  m_model->removeChannel(m_sample.data(), m_channel);
  delete m_channel;
}

