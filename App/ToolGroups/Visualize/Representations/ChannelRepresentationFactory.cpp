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

// ESPINA
#include <App/ToolGroups/Visualize/VisualizeToolGroup.h>
#include <App/ToolGroups/Visualize/Representations/ChannelRepresentationFactory.h>
#include <GUI/Representations/Pipelines/ChannelSlicePipeline.h>
#include <GUI/Representations/Pools/BufferedRepresentationPool.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/RepresentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::Support::Representations::Utils;

//----------------------------------------------------------------------------
ChannelRepresentationFactory::ChannelRepresentationFactory()
{
}

//----------------------------------------------------------------------------
Representation ChannelRepresentationFactory::createRepresentation(Support::Context &context) const
{
  Representation representation;

  createSliceRepresentation(representation, context);

  return representation;
}

//----------------------------------------------------------------------------
void ChannelRepresentationFactory::createSliceRepresentation(Representation &representation, Support::Context &context) const
{
  const unsigned WINDOW_SIZE = 10;

  auto scheduler      = context.scheduler();
  auto &timer         = context.timer();
  auto pipelineXY     = std::make_shared<ChannelSlicePipeline>(Plane::XY);
  auto pipelineXZ     = std::make_shared<ChannelSlicePipeline>(Plane::XZ);
  auto pipelineYZ     = std::make_shared<ChannelSlicePipeline>(Plane::YZ);
  auto poolXY         = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineXY, scheduler, WINDOW_SIZE);
  auto poolXZ         = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineXZ, scheduler, WINDOW_SIZE);
  auto poolYZ         = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineYZ, scheduler, WINDOW_SIZE);
  auto sliceManager   = std::make_shared<SliceManager>(poolXY, poolXZ, poolYZ);
  auto sliceSwitch    = std::make_shared<BasicRepresentationSwitch>(sliceManager, ViewType::VIEW_2D, timer);
  auto slice3DManager = std::make_shared<Slice3DManager>(poolXY, poolXZ, poolYZ);
  auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D, timer);

  sliceManager->setName(QObject::tr("Slice Representation"));
  sliceManager->setIcon(QIcon(":espina/channels_slice_switch.png"));
  sliceManager->setDescription(QObject::tr("Channel Slice Representation"));

  sliceSwitch->setActive(true);

  slice3DManager->setName(QObject::tr("Slice Representation"));
  slice3DManager->setIcon(QIcon(":espina/channels_slice3D_switch.svg"));
  slice3DManager->setDescription(QObject::tr("Channel 3D Slice Representation"));

  representation.Group     = CHANNELS_GROUP;
  representation.Pools    << poolXY << poolXZ << poolYZ;

  representation.Managers << sliceManager << slice3DManager;
  representation.Switches << sliceSwitch << slice3DSwitch;
}