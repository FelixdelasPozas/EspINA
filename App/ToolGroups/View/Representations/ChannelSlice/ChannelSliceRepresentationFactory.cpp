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

#include "ChannelSliceRepresentationFactory.h"

#include "ChannelSlicePipeline.h"

#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>

#include <ToolGroups/View/ViewToolGroup.h>
#include <GUI/Representations/BufferedRepresentationPool.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
ChannelSliceRepresentationFactory::ChannelSliceRepresentationFactory(SchedulerSPtr scheduler)
: m_scheduler{scheduler}
{
}

//----------------------------------------------------------------------------
Representation ChannelSliceRepresentationFactory::createRepresentation(ColorEngineSPtr colorEngine) const
{
  Representation representation;

  const unsigned WINDOW_SIZE = 10;

  auto pipelineXY     = std::make_shared<ChannelSlicePipeline>(Plane::XY);
  auto pipelineXZ     = std::make_shared<ChannelSlicePipeline>(Plane::XZ);
  auto pipelineYZ     = std::make_shared<ChannelSlicePipeline>(Plane::YZ);
  auto poolXY         = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineXY, m_scheduler, WINDOW_SIZE);
  auto poolXZ         = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineXZ, m_scheduler, WINDOW_SIZE);
  auto poolYZ         = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineYZ, m_scheduler, WINDOW_SIZE);
  auto sliceManager   = std::make_shared<SliceManager>(poolXY, poolXZ, poolYZ);
  auto sliceSwitch    = std::make_shared<BasicRepresentationSwitch>(sliceManager, ViewType::VIEW_2D);
  auto slice3DManager = std::make_shared<Slice3DManager>(poolXY, poolXZ, poolYZ);
  auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D);

  sliceManager->setName(QObject::tr("Slice Representation"));
  sliceManager->setIcon(QIcon(":espina/slice.png"));
  //sliceManager->show();

  slice3DManager->setName(QObject::tr("Slice Representation"));
  slice3DManager->setIcon(QIcon(":espina/show_planes.svg"));

  representation.Group     = ViewToolGroup::CHANNELS_GROUP;
  representation.Pools    << poolXY << poolXZ << poolYZ;
  representation.Managers << sliceManager << slice3DManager;
  representation.Switches << sliceSwitch << slice3DSwitch;

  return representation;
}