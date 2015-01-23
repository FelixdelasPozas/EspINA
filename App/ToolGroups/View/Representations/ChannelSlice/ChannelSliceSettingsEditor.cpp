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

#include "ChannelSliceSettingsEditor.h"
#include "App/ToolGroups/View/Representations/RepresentationSettings.h"

using namespace ESPINA;
using namespace ESPINA::Representations;

//----------------------------------------------------------------------------
ChannelSliceSettingsEditor::ChannelSliceSettingsEditor()
: m_opacity(0)
{
}

//----------------------------------------------------------------------------
void ChannelSliceSettingsEditor::setOpacity(double value)
{
  m_opacity = value;
}

//----------------------------------------------------------------------------
double ChannelSliceSettingsEditor::opacity() const
{
  return m_opacity;
}

//----------------------------------------------------------------------------
RepresentationPipeline::Settings ChannelSliceSettingsEditor::settings()
{
  RepresentationPipeline::Settings settings;

  settings.setValue<double>(OPACITY, m_opacity);

  return settings;
}
