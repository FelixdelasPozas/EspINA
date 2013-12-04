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
void Selection::set(ChannelAdapterList selection)
{
  //TODO
}


//----------------------------------------------------------------------------
void Selection::set(SegmentationAdapterList selection)
{
  auto previousSelection = segmentations();

  SegmentationAdapterList modifiedSegmentations;

  if (previousSelection != selection)
  {
    for (auto segmentation : previousSelection)
    {
      segmentation->setSelected(false);
      modifiedSegmentations << segmentation;
    }

    m_segmentations.clear();

    for (auto segmentation : selection)
    {
      segmentation->setSelected(true);

      m_segmentations << segmentation;

      if (modifiedSegmentations.contains(segmentation))
      {
        modifiedSegmentations.removeOne(segmentation);
      } else 
      {
        modifiedSegmentations << segmentation;
      }
    }
  }

  emit selectionStateChanged(modifiedSegmentations);
  emit selectionStateChanged();
}


//----------------------------------------------------------------------------
void Selection::set(ViewItemAdapterList selection)
{
  auto previousSelection = items();

  ChannelAdapterList      modifiedChannels;
  SegmentationAdapterList modifiedSegmentations;

  if (previousSelection != selection)
  {
    for (auto item : previousSelection)
    {
      item->setSelected(false);
      if (ItemAdapter::Type::CHANNEL == item->type())
      {
        modifiedChannels << dynamic_cast<ChannelAdapterPtr>(item);
      } else
      {
        modifiedSegmentations << dynamic_cast<SegmentationAdapterPtr>(item);
      }
    }

    m_channels.clear();
    m_segmentations.clear();

    //   qDebug() << "Selection Changed";
    for (auto item : selection)
    {
      item->setSelected(true);
      if (ItemAdapter::Type::CHANNEL == item->type())
      {
        auto channel = dynamic_cast<ChannelAdapterPtr>(item);

        m_channels << channel;

        if (modifiedChannels.contains(channel))
        {
          modifiedChannels.removeOne(channel);
        } else 
        {
          modifiedChannels << channel;
        }

      } else
      {
        auto segmentation = dynamic_cast<SegmentationAdapterPtr>(item);

        m_segmentations << segmentation;

        if (modifiedSegmentations.contains(segmentation))
        {
          modifiedSegmentations.removeOne(segmentation);
        } else 
        {
          modifiedSegmentations << segmentation;
        }
      }
      //     qDebug() << "-" << m_selection[i]->data().toString();
    }
  }

  emit selectionStateChanged(modifiedChannels);
  emit selectionStateChanged(modifiedSegmentations);
  emit selectionStateChanged();
}


//----------------------------------------------------------------------------
ViewItemAdapterList Selection::items() const
{
  ViewItemAdapterList selectedItems;

  for(auto channel : m_channels)
  {
    selectedItems << channel;
  }

  for(auto segmentation : m_segmentations)
  {
    selectedItems << segmentation;
  }

  return selectedItems;
}

//----------------------------------------------------------------------------
void Selection::clear()
{
  m_channels.clear();
  m_segmentations.clear();
}
