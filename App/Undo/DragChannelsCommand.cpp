/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Channel.h>
#include <GUI/Model/Proxies/ChannelProxy.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/DragChannelsCommand.h>

using namespace ESPINA;

//------------------------------------------------------------------------
DragChannelsCommand::DragChannelsCommand(ModelAdapterSPtr model, ChannelAdapterList channels, SampleAdapterSPtr sample, ChannelProxy *proxy)
: QUndoCommand()
, m_model   {model}
, m_sample  {sample}
, m_proxy   {proxy}
{
  // NOTE: no check that channel's current sample is different than m_sample. That responsibility is elsewhere.
  for(auto channel: channels)
  {
    m_channels.insert(channel, QueryAdapter::sample(channel));
  }
}

//------------------------------------------------------------------------
void DragChannelsCommand::redo()
{
  for(auto channel: m_channels.keys())
  {
    move(channel, m_channels[channel], m_sample);
  }

  emitSignals();
}

//------------------------------------------------------------------------
void DragChannelsCommand::undo()
{
  for(auto channel: m_channels.keys())
  {
    move(channel, m_sample, m_channels[channel]);
  }

  emitSignals();
}

//------------------------------------------------------------------------
void DragChannelsCommand::emitSignals()
{
  ItemAdapterList items;
  auto modelSamples = m_model->samples();

  if(modelSamples.contains(m_sample))
  {
    items << m_sample.get();
  }

  for(auto channel: m_channels.keys())
  {
    if(modelSamples.contains(m_channels[channel]))
    {
      items << m_channels[channel].get();
    }

    items << channel;
  }

  for(auto item: items)
  {
    m_proxy->emitModified(item);
  }
}

//------------------------------------------------------------------------
void DragChannelsCommand::move(ChannelAdapterPtr channel, SampleAdapterSPtr from, SampleAdapterSPtr to)
{
  auto samples = m_model->samples();
  if(!samples.contains(to))
  {
    m_model->add(to);
  }

  auto sChannel = m_model->smartPointer(channel);
  m_model->deleteRelation(from, sChannel, Channel::STAIN_LINK);
  m_model->addRelation   (to,   sChannel, Channel::STAIN_LINK);

  auto channelsList = QueryAdapter::channels(from);
  if(channelsList.isEmpty())
  {
    m_model->remove(from);
  }
}
