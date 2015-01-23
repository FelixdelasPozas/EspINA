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

#include "ChannelSliceRepresentationDriver.h"

#include "ChannelSliceSwitch.h"
#include "ChannelSliceSettingsEditor.h"
#include "ChannelSlicePipeline.h"
#include "ChannelSliceManager.h"

#include <ToolGroups/View/ViewToolGroup.h>
#include <GUI/Representations/BasicRepresentationPool.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
ChannelSliceRepresentationDriver::ChannelSliceRepresentationDriver(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
{
}

//----------------------------------------------------------------------------
RepresentationDriver ChannelSliceRepresentationDriver::createRepresentationDriver() const
{
  RepresentationDriver driver;


  auto settings     = std::make_shared<ChannelSliceSettingsEditor>();
  auto poolXY       = std::make_shared<BasicRepresentationPool<ChannelSlicePipeline<Plane::XY>, ChannelSliceSettingsEditorSPtr>>(settings, m_scheduler);
  auto poolXZ       = std::make_shared<BasicRepresentationPool<ChannelSlicePipeline<Plane::XZ>, ChannelSliceSettingsEditorSPtr>>(settings, m_scheduler);
  auto poolYZ       = std::make_shared<BasicRepresentationPool<ChannelSlicePipeline<Plane::YZ>, ChannelSliceSettingsEditorSPtr>>(settings, m_scheduler);
  auto sliceManager = std::make_shared<ChannelSliceManager>(poolXY, poolXZ, poolYZ);
  auto sliceSwitch  = std::make_shared<ChannelSliceSwitch>(sliceManager);

  driver.Group  = ViewToolGroup::CHANNELS_GROUP;
  driver.Pools    << poolXY << poolXZ << poolYZ;
  driver.Managers << sliceManager;
  driver.Switches << sliceSwitch;

  return driver;
}
