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

#ifndef ESPINA_SELECTION_H
#define ESPINA_SELECTION_H

#include "GUI/EspinaGUI_Export.h"

#include <QObject>

#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>

namespace EspINA 
{

  class EspinaGUI_EXPORT Selection
  : public QObject
  {
    Q_OBJECT

  public:
    void set(ChannelAdapterList      selection);
    void set(SegmentationAdapterList selection);
    void set(ViewItemAdapterList     selection);

    ChannelAdapterList channels() const
    { return m_channels; }

    SegmentationAdapterList segmentations() const
    { return m_segmentations; }

    ViewItemAdapterList items() const;

    void clear();

  signals:
    void selectionStateChanged(ChannelAdapterList);

    void selectionStateChanged(SegmentationAdapterList);

    void selectionStateChanged();

  private:
    ChannelAdapterList setChannels(ChannelAdapterList);
    SegmentationAdapterList setSegmentations(SegmentationAdapterList);

    ChannelAdapterList      m_channels;
    SegmentationAdapterList m_segmentations;
  };

  using SelectionSPtr = std::shared_ptr<Selection>;
}

#endif // ESPINA_SELECTION_H
