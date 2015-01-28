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

#include "ChannelRepresentationSettingsEditor.h"

#include "App/ToolGroups/View/Representations/RepresentationSettings.h"

#include <vtkType.h>
#include <QColor>

using namespace ESPINA;
using namespace ESPINA::Representations;

//----------------------------------------------------------------------------
ChannelRepresentationSettingsEditor::ChannelRepresentationSettingsEditor(ChannelAdapterPtr channel)
: m_channel{channel}
{
}

// //----------------------------------------------------------------------------
// bool ChannelRepresentationSettingsEditor::needUpdate(TimeStamp timeStamp)
// {
//   bool requestedVisibility = m_channel->isVisible();
//   bool visibilityChanged   = m_visible != requestedVisibility;
//
//   m_visible = requestedVisibility;
//
//   bool brightnessChanged = false;
//   bool contrastChanged   = false;
//   bool opacityChanged    = false;
//   bool stainChanged      = false;
//   bool outputChanged     = false;
//
//   double hue = -1.0 == m_channel->hue() ? 0 : m_channel->hue();
//   double sat = -1.0 == m_channel->hue() ? 0 : m_channel->saturation();
//
//   if (m_visible)
//   {
//     double    requestedBrightness = m_channel->brightness();
//     double    requestedContrast   = m_channel->contrast();
//     double    requestedOpacity    = m_channel->opacity();
//     QColor    requestedStain      = QColor::fromHsvF(hue, sat, 1.0);
//     TimeStamp requestedTimeStamp  = timeStamp;
//
//     Q_ASSERT(m_channel->output());
//
//     outputChanged     = m_output     != m_channel->output() || m_timeStamp != requestedTimeStamp;
//     brightnessChanged = m_brightness != requestedBrightness || outputChanged;
//     contrastChanged   = m_contrast   != requestedContrast   || outputChanged;
//     opacityChanged    = m_opacity    != requestedOpacity    || outputChanged;
//     stainChanged      = m_stain      != requestedStain      || outputChanged;
//   }
//
//   return visibilityChanged
//       || brightnessChanged
//       || contrastChanged
//       || opacityChanged
//       || stainChanged;
// }

//----------------------------------------------------------------------------
RepresentationPipeline::Settings ChannelRepresentationSettingsEditor::settings()
{
  RepresentationPipeline::Settings settings;

  double hue   = -1.0 == m_channel->hue() ? 0 : m_channel->hue();
  double sat   = -1.0 == m_channel->hue() ? 0 : m_channel->saturation();

  settings.setValue<double>(BRIGHTNESS, m_channel->brightness());
  settings.setValue<double>(CONTRAST,   m_channel->contrast());
  settings.setValue<double>(OPACITY,    m_channel->opacity());
  settings.setValue<double>(VISIBLE,    m_channel->isVisible());
  settings.setValue<QColor>(STAIN,      QColor::fromHsvF(hue, sat, 1.0));

  return settings;
}

// //----------------------------------------------------------------------------
// void ChannelRepresentationSettingsEditor::apply(RepresentationPipeline::Settings &settings)
// {
//
//   m_brightness = m_channel->brightness();
//   m_contrast   = m_channel->contrast();
//   m_opacity    = m_channel->opacity();
//   m_output     = m_channel->output();
//   m_stain      = QColor::fromHsvF(hue, sat, 1.0);
//   //m_timeStamp  = timeStamp;
// }