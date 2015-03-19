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

#include "ChannelPipeline.h"

#include "App/ToolGroups/1_Explore/Representations/RepresentationSettings.h"

#include <vtkType.h>
#include <QColor>

using namespace ESPINA;
using namespace ESPINA::Representations;

//----------------------------------------------------------------------------
RepresentationState ChannelPipeline::Settings(ChannelAdapterPtr channel)
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
double ESPINA::brightness(const RepresentationState &state)
{
  return state.getValue<double>(BRIGHTNESS);
}

//----------------------------------------------------------------------------
double ESPINA::contrast(const RepresentationState &state)
{
  return state.getValue<double>(CONTRAST);
}

//----------------------------------------------------------------------------
QColor ESPINA::stain(const RepresentationState &state)
{
  return state.getValue<QColor>(STAIN);
}
