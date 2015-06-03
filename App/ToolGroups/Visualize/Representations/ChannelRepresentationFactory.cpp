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
using namespace ESPINA::Support;
using namespace ESPINA::Support::Representations::Utils;

//----------------------------------------------------------------------------
ChannelRepresentationFactory::ChannelRepresentationFactory()
{
}

//----------------------------------------------------------------------------
Representation ChannelRepresentationFactory::doCreateRepresentation(Context &context, ViewTypeFlags supportedViews) const
{
  Representation representation;

  createSliceRepresentation(representation, context, supportedViews);

  return representation;
}

//----------------------------------------------------------------------------
void ChannelRepresentationFactory::createSliceRepresentation(Representation &representation, Context &context, ViewTypeFlags supportedViews) const
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

  representation.Group     = CHANNELS_GROUP;
  representation.Pools    << poolXY << poolXZ << poolYZ;

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    auto sliceManager   = std::make_shared<SliceManager>(poolXY, poolXZ, poolYZ);
    sliceManager->setName(QObject::tr("Channel Slice Representation"));
    sliceManager->setIcon(QIcon(":espina/channels_slice2D_switch.svg"));
    sliceManager->setDescription(QObject::tr("Channel Slice Representation"));

    auto sliceSwitch = std::make_shared<BasicRepresentationSwitch>(sliceManager, ViewType::VIEW_2D, timer, context);
    sliceSwitch->setChecked(true);
    sliceSwitch->setGroupWith("channel_reps");

    representation.Managers << sliceManager;
    representation.Switches << sliceSwitch;
  }

  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    auto slice3DManager = std::make_shared<Slice3DManager>(poolXY, poolXZ, poolYZ);
    slice3DManager->setName(QObject::tr("Slice Representation"));
    slice3DManager->setIcon(QIcon(":espina/channels_slice3D_switch.svg"));
    slice3DManager->setDescription(QObject::tr("Channel 3D Slice Representation"));

    auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D, timer, context);
    slice3DSwitch->setGroupWith("channel_reps");

    representation.Managers << slice3DManager;
    representation.Switches << slice3DSwitch;
  }
}