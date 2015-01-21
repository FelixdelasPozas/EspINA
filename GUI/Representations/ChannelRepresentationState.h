/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CHANNEL_STATE_H
#define ESPINA_CHANNEL_STATE_H

#include <GUI/Model/ChannelAdapter.h>

namespace ESPINA {

  class ChannelRepresentationState
  {
  public:
    using Item = ChannelAdapterPtr;

  public:
    explicit ChannelRepresentationState();

    explicit ChannelRepresentationState(Item item);

    bool updateState();

  private:
    Item       m_item;

    double     m_brightness;
    double     m_contrast;
    double     m_opacity;
    QColor     m_stain;
    bool       m_visible;
    TimeStamp  m_timeStamp;
    OutputSPtr m_output;

    RepresentationSList representations;
  };

}

#endif // ESPINA_CHANNELSTATE_H
