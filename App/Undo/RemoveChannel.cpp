/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "RemoveChannel.h"
#include <GUI/Model/Utils/QueryAdapter.h>

using namespace ESPINA;

//------------------------------------------------------------------------
RemoveChannel::RemoveChannel(ChannelAdapterSPtr channel, Support::Context &context, QUndoCommand* parent)
: QUndoCommand{parent}
, m_channel   {channel}
, m_sample    {nullptr}
, m_relations {context.model()->relations(channel.get(), ESPINA::RELATION_INOUT)}
, m_context   (context)
, m_active    {Support::getActiveChannel(context) == m_channel.get()}
{
}

//------------------------------------------------------------------------
RemoveChannel::~RemoveChannel()
{
}

//------------------------------------------------------------------------
void RemoveChannel::redo()
{
  auto sample    = QueryAdapter::sample(m_channel);
  auto channels  = QueryAdapter::channels(sample);
  auto model     = m_context.model();

  Q_ASSERT(sample != nullptr && !channels.isEmpty());
  model->remove(m_channel);

  if(channels.size() == 1)
  {
    m_sample = sample;
    model->remove(sample);
  }

  if(m_active)
  {
    auto nextChannel = m_context.model()->channels().first();
    Support::getSelection(m_context)->setActiveChannel(nextChannel.get());
  }
}

//------------------------------------------------------------------------
void RemoveChannel::undo()
{
  auto model = m_context.model();

  if(m_sample != nullptr)
  {
    model->add(m_sample);
  }

  model->add(m_channel);

  for(auto relation: m_relations)
  {
    model->addRelation(relation.ancestor, relation.successor, relation.relation);
  }

  if(m_active)
  {
    Support::getSelection(m_context)->setActiveChannel(m_channel.get());
  }
}
