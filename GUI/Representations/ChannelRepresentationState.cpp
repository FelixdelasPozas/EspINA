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

#include "ChannelRepresentationState.h"

using namespace ESPINA;

//----------------------------------------------------------------------------
ChannelRepresentationState::ChannelRepresentationState()
: ChannelRepresentationState(nullptr)
{
}

//----------------------------------------------------------------------------
ChannelRepresentationState::ChannelRepresentationState(Item item)
: m_item{item}
, m_brightness{0}
, m_contrast{0}
, m_opacity{0}
, m_stain{Qt::red}
, m_visible{item?!item->isVisible():false} // ensures first update
, m_timeStamp{0}
{
}

//----------------------------------------------------------------------------
bool ChannelRepresentationState::updateState()
{
  bool requestedVisibility = m_item->isVisible();

  bool visibilityChanged = m_visible != requestedVisibility;
  m_visible = requestedVisibility;

  bool brightnessChanged = false;
  bool contrastChanged   = false;
  bool opacityChanged    = false;
  bool stainChanged      = false;
  bool outputChanged     = false;

  double hue = -1.0 == m_item->hue() ? 0 : m_item->hue();
  double sat = -1.0 == m_item->hue() ? 0 : m_item->saturation();

  if (m_visible)
  {
    double    requestedBrightness = m_item->brightness();
    double    requestedContrast   = m_item->contrast();
    double    requestedOpacity    = m_item->opacity();
    QColor    requestedStain      = QColor::fromHsvF(hue, sat, 1.0);
    TimeStamp requestedTimeStamp  = m_item->output()->lastModified();

    Q_ASSERT(m_item->output());
    if (!m_output)
    {
      m_timeStamp = m_item->output()->lastModified();
    }

    outputChanged     = m_output     != m_item->output()    || m_timeStamp != requestedTimeStamp;
    brightnessChanged = m_brightness != requestedBrightness || outputChanged;
    contrastChanged   = m_contrast   != requestedContrast   || outputChanged;
    opacityChanged    = m_opacity    != requestedOpacity    || outputChanged;
    stainChanged      = m_stain      != requestedStain      || outputChanged;

    m_brightness  = requestedBrightness;
    m_contrast    = requestedContrast;
    m_opacity     = requestedOpacity;
    m_stain       = requestedStain;
    m_output      = m_item->output();
    m_timeStamp   = requestedTimeStamp;
  }

  bool hasChanged = visibilityChanged || brightnessChanged || contrastChanged || opacityChanged || stainChanged;

  // TODO emit signals

  return hasChanged;
}
