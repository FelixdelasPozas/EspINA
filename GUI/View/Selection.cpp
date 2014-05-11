/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "Selection.h"

#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/SegmentationAdapter.h"

using namespace EspINA;

//----------------------------------------------------------------------------
ChannelAdapterList Selection::setChannels(ChannelAdapterList channelList)
{
  auto modifiedChannels = channels();
  m_channels.clear();

  for (auto channel : modifiedChannels)
    channel->setSelected(false);

  for(auto channel: channelList)
  {
    if(modifiedChannels.contains(channel))
      modifiedChannels.removeOne(channel);
    else
      modifiedChannels << channel;

    channel->setSelected(true);
    m_channels << channel;
  }

  return modifiedChannels;
}

//----------------------------------------------------------------------------
void Selection::set(ChannelAdapterList selection)
{
  if(selection != m_channels)
  {
    auto modifiedChannels = setChannels(selection);

    if(!modifiedChannels.empty())
      emit selectionStateChanged(modifiedChannels);

    emit selectionStateChanged();
  }
}

//----------------------------------------------------------------------------
SegmentationAdapterList Selection::setSegmentations(SegmentationAdapterList segmentationList)
{
  auto modifiedSegmentations = segmentations();
  m_segmentations.clear();

  for(auto segmentation: modifiedSegmentations)
    segmentation->setSelected(false);

  for(auto segmentation: segmentationList)
  {
    if (modifiedSegmentations.contains(segmentation))
      modifiedSegmentations.removeOne(segmentation);
    else
      modifiedSegmentations << segmentation;

    segmentation->setSelected(true);
    m_segmentations << segmentation;
  }

  return modifiedSegmentations;
}

//----------------------------------------------------------------------------
void Selection::set(SegmentationAdapterList selection)
{
  if(selection != m_segmentations)
  {
    auto modifiedSegmentations = setSegmentations(selection);

    if(!modifiedSegmentations.empty())
      emit selectionStateChanged(modifiedSegmentations);

    emit selectionStateChanged();
  }
}


//----------------------------------------------------------------------------
void Selection::set(ViewItemAdapterList selection)
{
  ChannelAdapterList      channels;
  SegmentationAdapterList segmentations;

  if(selection != items())
  {
    for(auto item: selection)
      switch(item->type())
      {
        case ItemAdapter::Type::CHANNEL:
          channels << dynamic_cast<ChannelAdapterPtr>(item);
          break;
        case ItemAdapter::Type::SEGMENTATION:
          segmentations << dynamic_cast<SegmentationAdapterPtr>(item);
          break;
        default:
          Q_ASSERT(false); // TODO: SAMPLES?
          break;
      }

    auto modifiedSegmentations = setSegmentations(segmentations);
    auto modifiedChannels = setChannels(channels);

    if(!modifiedSegmentations.empty())
      emit selectionStateChanged(modifiedSegmentations);

    if(!modifiedChannels.empty())
      emit selectionStateChanged(modifiedChannels);

    emit selectionStateChanged();
  }
}

//----------------------------------------------------------------------------
ViewItemAdapterList Selection::items() const
{
  ViewItemAdapterList selectedItems;

  for(auto channel: m_channels)
    selectedItems << channel;

  for(auto segmentation: m_segmentations)
    selectedItems << segmentation;

  return selectedItems;
}

//----------------------------------------------------------------------------
void Selection::clear()
{
  for(auto channel: m_channels)
    channel->setSelected(false);

  m_channels.clear();

  for(auto segmentation: m_segmentations)
    segmentation->setSelected(false);

  m_segmentations.clear();
}
