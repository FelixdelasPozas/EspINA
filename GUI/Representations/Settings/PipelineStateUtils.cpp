/*

    Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/Settings/PipelineStateUtils.h>

using namespace ESPINA::Representations;

namespace ESPINA
{
  //----------------------------------------------------------------------------
  RepresentationState channelPipelineSettings(ChannelAdapterPtr channel)
  {
    RepresentationState settings;

    double hue   = -1.0 == channel->hue() ? 0 : channel->hue();
    double sat   = -1.0 == channel->hue() ? 0 : channel->saturation();

    settings.setValue<double>(BRIGHTNESS, channel->brightness());
    settings.setValue<double>(CONTRAST,   channel->contrast());
    settings.setValue<double>(OPACITY,    channel->opacity());
    settings.setValue<double>(VISIBLE,    channel->isVisible());
    settings.setValue<QColor>(STAIN,      QColor::fromHsvF(hue, sat, 1.0));

    return settings;
  }

  //----------------------------------------------------------------------------
  RepresentationState segmentationPipelineSettings(SegmentationAdapterPtr segmentation)
  {
    RepresentationState settings;

    settings.setValue<double>(VISIBLE, segmentation->isVisible());

    return settings;
  }

  //----------------------------------------------------------------------------
  double brightness(const RepresentationState &state)
  {
    if(!state.hasValue(BRIGHTNESS)) return 0;

    return state.getValue<double>(BRIGHTNESS);
  }

  //----------------------------------------------------------------------------
  double contrast(const RepresentationState &state)
  {
    if(!state.hasValue(CONTRAST)) return 0;

    return state.getValue<double>(CONTRAST);
  }

  //----------------------------------------------------------------------------
  double opacity(const RepresentationState &state)
  {
    if(!state.hasValue(OPACITY)) return 1;

    return state.getValue<double>(OPACITY);
  }

  //----------------------------------------------------------------------------
  QColor stain(const RepresentationState &state)
  {
    if(!state.hasValue(STAIN)) return QColor(0,0,0);

    return state.getValue<QColor>(STAIN);
  }
}

