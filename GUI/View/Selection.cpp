/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include <Core/Utils/ListUtils.hxx>
#include "Selection.h"
#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/SegmentationAdapter.h"
#include <GUI/Model/Utils/SegmentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::View;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
Selection::Selection()
: m_activeChannel{nullptr}
{
}

//----------------------------------------------------------------------------
ChannelAdapterList Selection::setChannels(ChannelAdapterList channelList)
{
  auto modifiedChannels = m_channels;
  modifiedChannels.detach();
  m_channels.clear();

  for (auto channel : modifiedChannels)
  {
    channel->setSelected(false);
  }

  for(auto channel: channelList)
  {
    if(modifiedChannels.contains(channel))
    {
      modifiedChannels.removeOne(channel);
    }
    else
    {
      modifiedChannels << channel;
    }

    channel->setSelected(true);
    m_channels << channel;
  }

  return modifiedChannels;
}

//----------------------------------------------------------------------------
SegmentationAdapterList Selection::setSegmentations(SegmentationAdapterList segmentationList)
{
  auto modifiedSegmentations = m_segmentations;
  modifiedSegmentations.detach();
  m_segmentations.clear();

  for(auto segmentation: modifiedSegmentations)
  {
    segmentation->setSelected(false);
  }

  for(auto segmentation: segmentationList)
  {
    if (modifiedSegmentations.contains(segmentation))
    {
      modifiedSegmentations.removeOne(segmentation);
    }
    else
    {
      modifiedSegmentations << segmentation;
    }

    segmentation->setSelected(true);
    m_segmentations << segmentation;
  }

  return modifiedSegmentations;
}

//----------------------------------------------------------------------------
void Selection::onChannelsModified(ChannelAdapterList channels)
{
  if(!channels.isEmpty())
  {
    emit selectionStateChanged(channels);
  }
}

//----------------------------------------------------------------------------
void Selection::onSegmentationsModified(SegmentationAdapterList segmentations)
{
  if(!segmentations.isEmpty())
  {
    emit selectionStateChanged(segmentations);
  }
}

//----------------------------------------------------------------------------
void Selection::set(ChannelAdapterList selection)
{
  ChannelAdapterList modifiedChannels;
  bool modified = false;

  {
    QMutexLocker lock(&m_mutex);

    if(selection != m_channels)
    {
      modifiedChannels = setChannels(selection);
      modified = true;
    }
  }

  if(modified)
  {
    onChannelsModified(modifiedChannels);

    emit selectionStateChanged();

    emit selectionChanged(m_channels);
    emit selectionChanged();
  }
}

//----------------------------------------------------------------------------
void Selection::set(SegmentationAdapterList selection)
{
  SegmentationAdapterList modifiedSegmentations;
  bool modified = false;

  {
    QMutexLocker lock(&m_mutex);

    if(selection != m_segmentations)
    {
      modifiedSegmentations = setSegmentations(selection);
      modified = true;
    }
  }

  if(modified)
  {
    onSegmentationsModified(modifiedSegmentations);

    emit selectionStateChanged();

    emit selectionChanged(m_segmentations);
    emit selectionChanged();
  }
}

//----------------------------------------------------------------------------
void Selection::setItems(ViewItemAdapterList list)
{
  ChannelAdapterList      channels, modifiedChannels;
  SegmentationAdapterList segmentations, modifiedSegmentations;

  if(list != items_implementation())
  {
    for(auto item: list)
    {
      switch(item->type())
      {
        case ItemAdapter::Type::CHANNEL:
          channels << channelPtr(item);
          break;
        case ItemAdapter::Type::SEGMENTATION:
          segmentations << segmentationPtr(item);
          break;
        default:
          Q_ASSERT(false); // NOTE: SAMPLES?
          break;
      }
    }

    {
      QMutexLocker lock(&m_mutex);
      modifiedChannels      = setChannels(channels);
      modifiedSegmentations = setSegmentations(segmentations);
    }

    if (!modifiedChannels.empty() || !modifiedSegmentations.empty())
    {
      emit selectionStateChanged();
    }

    onChannelsModified(modifiedChannels);
    onSegmentationsModified(modifiedSegmentations);

    emit selectionChanged();
    emit selectionChanged(m_channels);
    emit selectionChanged(m_segmentations);
  }
}

//----------------------------------------------------------------------------
void Selection::set(ViewItemAdapterList selection)
{
  setItems(selection);
}

//----------------------------------------------------------------------------
void Selection::unset(ViewItemAdapterList itemList)
{
  ViewItemAdapterList toDeselect;
  auto selection = items();

  for(auto item: selection)
  {
    if(itemList.contains(item))
    {
      toDeselect << item;
    }
  }

  if(!toDeselect.isEmpty())
  {
    for(auto item: toDeselect)
    {
      selection.removeOne(item);
    }

    setItems(selection);
  }
}

//----------------------------------------------------------------------------
void Selection::setActiveChannel(ChannelAdapterPtr channel)
{
  bool modified = false;
  {
    QMutexLocker lock(&m_mutex);

    if (m_activeChannel != channel)
    {
      m_activeChannel = channel;
      modified = true;
    }
  }

  if(modified)
  {
    emit activeChannelChanged(channel);
  }
}

//----------------------------------------------------------------------------
ChannelAdapterPtr Selection::activeChannel() const
{
  QMutexLocker lock(&m_mutex);

  return m_activeChannel;
}

//----------------------------------------------------------------------------
ViewItemAdapterList Selection::items_implementation() const
{
  ViewItemAdapterList selectedItems;

  for(auto channel: m_channels)
  {
    selectedItems << channel;
  }

  for(auto segmentation: m_segmentations)
  {
    selectedItems << segmentation;
  }

  return selectedItems;
}

//----------------------------------------------------------------------------
ViewItemAdapterList Selection::items() const
{
  QMutexLocker lock(&m_mutex);

  return items_implementation();
}

//----------------------------------------------------------------------------
void Selection::clear()
{
  setItems(ViewItemAdapterList());
}

//----------------------------------------------------------------------------
void Selection::modified()
{
  emit selectionStateChanged();
}

//----------------------------------------------------------------------------
ChannelAdapterList Selection::channels() const
{
  QMutexLocker lock(&m_mutex);

  auto channels = m_channels;
  channels.detach();

  return channels;
}

//----------------------------------------------------------------------------
SegmentationAdapterList Selection::segmentations() const
{
  QMutexLocker lock(&m_mutex);

  auto segmentations = m_segmentations;
  segmentations.detach();

  return segmentations;
}
