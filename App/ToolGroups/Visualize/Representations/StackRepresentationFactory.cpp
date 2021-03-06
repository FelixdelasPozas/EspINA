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
#include <GUI/Representations/Pipelines/ChannelSlicePipeline.h>
#include <GUI/Representations/Pools/BufferedRepresentationPool.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/RepresentationUtils.h>
#include <ToolGroups/Visualize/Representations/StackRepresentationFactory.h>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Representations::Utils;

const unsigned StackRepresentationFactory::WINDOW_SIZE = 8;

//----------------------------------------------------------------------------
StackRepresentationFactory::StackRepresentationFactory()
{
}

//----------------------------------------------------------------------------
Representation StackRepresentationFactory::doCreateRepresentation(Context &context, ViewTypeFlags supportedViews) const
{
  Representation representation;

  createSliceRepresentation(representation, context, supportedViews);

  return representation;
}

//----------------------------------------------------------------------------
void StackRepresentationFactory::createSliceRepresentation(Representation &representation, Context &context, ViewTypeFlags supportedViews) const
{
  auto scheduler  = context.scheduler();

  auto pipelineXY = std::make_shared<ChannelSlicePipeline>(Plane::XY);
  auto pipelineXZ = std::make_shared<ChannelSlicePipeline>(Plane::XZ);
  auto pipelineYZ = std::make_shared<ChannelSlicePipeline>(Plane::YZ);
  auto poolXY     = std::make_shared<BufferedRepresentationPool>(ItemAdapter::Type::CHANNEL, Plane::XY, pipelineXY, scheduler, WINDOW_SIZE);
  auto poolXZ     = std::make_shared<BufferedRepresentationPool>(ItemAdapter::Type::CHANNEL, Plane::XZ, pipelineXZ, scheduler, WINDOW_SIZE);
  auto poolYZ     = std::make_shared<BufferedRepresentationPool>(ItemAdapter::Type::CHANNEL, Plane::YZ, pipelineYZ, scheduler, WINDOW_SIZE);

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    auto sliceManager   = std::make_shared<SliceManager>(poolXY, poolXZ, poolYZ);
    sliceManager->setName("DisplayStacks");
    sliceManager->setIcon(QIcon(":espina/display_stacks.svg"));
    sliceManager->setDescription(QObject::tr("Display Stacks"));

    auto sliceSwitch = std::make_shared<BasicRepresentationSwitch>("DisplayStacks", sliceManager, ViewType::VIEW_2D, context);
    sliceSwitch->setChecked(true);
    sliceSwitch->setOrder("0-0", "0-Representations");

    representation.Managers << sliceManager;
    representation.Switches << sliceSwitch;
  }

  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    auto slice3DManager = std::make_shared<Slice3DManager>(poolXY, poolXZ, poolYZ);
    slice3DManager->setName("DisplayStackCrosshairs");
    slice3DManager->setIcon(QIcon(":espina/display_stacks.svg"));
    slice3DManager->setDescription(QObject::tr("Display Stacks"));

    auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>("DisplayStackCrosshairs", slice3DManager, ViewType::VIEW_3D, context);
    slice3DSwitch->setOrder("0-0", "0-Representations");

    representation.Group    = STACKS_GROUP;
    representation.Pools    << poolXY << poolXZ << poolYZ;
    representation.Managers << slice3DManager;
    representation.Switches << slice3DSwitch;
  }
}
