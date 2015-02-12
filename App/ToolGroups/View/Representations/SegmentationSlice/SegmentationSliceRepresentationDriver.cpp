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

#include "SegmentationSliceRepresentationDriver.h"

#include "SegmentationSliceSettings.h"
#include "SegmentationSlicePipeline.h"
#include "SegmentationSliceSwitch.h"

#include <ToolGroups/View/ViewToolGroup.h>
#include <GUI/Representations/BufferedRepresentationPool.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/BasicRepresentationSwitch.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
SegmentationSliceRepresentationDriver::SegmentationSliceRepresentationDriver(SchedulerSPtr scheduler)
: m_scheduler(scheduler)
{

}

//----------------------------------------------------------------------------
Representation SegmentationSliceRepresentationDriver::createRepresentation() const
{
  const unsigned WINDOW_SIZE = 10;

  Representation representation;

  auto settings     = std::make_shared<SegmentationSliceSettings>();
  auto poolXY       = std::make_shared<BufferedRepresentationPool<SegmentationSlicePipeline<Plane::XY>>>(Plane::XY, m_scheduler, WINDOW_SIZE);
  auto poolXZ       = std::make_shared<BufferedRepresentationPool<SegmentationSlicePipeline<Plane::XZ>>>(Plane::XZ, m_scheduler, WINDOW_SIZE);
  auto poolYZ       = std::make_shared<BufferedRepresentationPool<SegmentationSlicePipeline<Plane::YZ>>>(Plane::YZ, m_scheduler, WINDOW_SIZE);
  auto sliceManager = std::make_shared<SliceManager>(poolXY, poolXZ, poolYZ);
  auto sliceSwitch  = std::make_shared<SegmentationSliceSwitch>(sliceManager);
  auto slice3DManager = std::make_shared<Slice3DManager>(poolXY, poolXZ, poolYZ);
  auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D);

  configurePool(poolXY, settings);
  configurePool(poolXZ, settings);
  configurePool(poolYZ, settings);

  sliceManager->setName(QObject::tr("Slice Representation"));

  slice3DManager->setName(QObject::tr("Slice Representation"));
  slice3DManager->setIcon(QIcon(":espina/show_planes.svg"));

  representation.Group     = ViewToolGroup::SEGMENTATIONS_GROUP;
  representation.Pools    << poolXY << poolXZ << poolYZ;
  representation.Managers << sliceManager << slice3DManager;
  representation.Switches << sliceSwitch << slice3DSwitch;

  return representation;
}

//----------------------------------------------------------------------------
void SegmentationSliceRepresentationDriver::configurePool(RepresentationPoolSPtr           pool,
                                                          RepresentationPool::SettingsSPtr settings) const
{
  pool->setSettings(settings);
}
