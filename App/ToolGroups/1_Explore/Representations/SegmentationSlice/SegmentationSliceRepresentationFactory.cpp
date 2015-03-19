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

#include "SegmentationSliceRepresentationFactory.h"

#include "SegmentationSliceSettings.h"
#include "SegmentationSlicePipeline.h"
#include "SegmentationSliceSwitch.h"

#include <GUI/Representations/BasicRepresentationPool.h>
#include <GUI/Representations/BufferedRepresentationPool.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <ToolGroups/1_Explore//Representations/SegmentationVolumetric/SegmentationVolumetricPipeline.h>
#include <ToolGroups/1_Explore//Representations/SegmentationVolumetric/VolumetricManager.h>
#include <ToolGroups/1_Explore/ExploreToolGroup.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
SegmentationSliceRepresentationFactory::SegmentationSliceRepresentationFactory(SchedulerSPtr scheduler)
: m_scheduler(scheduler)
{

}

//----------------------------------------------------------------------------
Representation ESPINA::SegmentationSliceRepresentationFactory::createRepresentation(ColorEngineSPtr colorEngine) const
{
  const unsigned WINDOW_SIZE = 5;

  Representation representation;

  auto settings       = std::make_shared<SegmentationSliceSettings>();
  auto pipelineXY     = std::make_shared<SegmentationSlicePipeline>(Plane::XY, colorEngine);
  auto pipelineXZ     = std::make_shared<SegmentationSlicePipeline>(Plane::XZ, colorEngine);
  auto pipelineYZ     = std::make_shared<SegmentationSlicePipeline>(Plane::YZ, colorEngine);
  auto poolXY         = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineXY, m_scheduler, WINDOW_SIZE);
  auto poolXZ         = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineXZ, m_scheduler, WINDOW_SIZE);
  auto poolYZ         = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineYZ, m_scheduler, WINDOW_SIZE);
  auto sliceManager   = std::make_shared<SliceManager>();
  sliceManager->addPool(poolXY, Plane::XY);
  sliceManager->addPool(poolXZ, Plane::XZ);
  sliceManager->addPool(poolYZ, Plane::YZ);
  auto sliceSwitch    = std::make_shared<BasicRepresentationSwitch>(sliceManager, ViewType::VIEW_2D);
  auto slice3DManager = std::make_shared<Slice3DManager>();
  slice3DManager->addPool(poolXY);
  slice3DManager->addPool(poolXZ);
  slice3DManager->addPool(poolYZ);
  auto slice3DSwitch  = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D);

  // Volume
  auto pipelineVolume = std::make_shared<SegmentationVolumetricPipeline>(colorEngine);
  auto poolVolumetric = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineVolume);
  configurePool(poolVolumetric, colorEngine, settings);

  auto volumetricManager = std::make_shared<VolumetricManager>();
  volumetricManager->setName(QObject::tr("Volumetric Representation"));
  volumetricManager->setIcon(QIcon(":espina/voxel.png"));
  volumetricManager->addPool(poolVolumetric);
  auto volumetricSwitch = std::make_shared<BasicRepresentationSwitch>(volumetricManager, ViewType::VIEW_3D);
  // ** end volume

  configurePool(poolXY, colorEngine, settings);
  configurePool(poolXZ, colorEngine, settings);
  configurePool(poolYZ, colorEngine, settings);

  sliceManager->setName(QObject::tr("Slice Representation"));
  sliceManager->setIcon(QIcon(":espina/segmentations_slice_switch.svg"));

  sliceSwitch->setActive(true);

  slice3DManager->setName(QObject::tr("Slice Representation"));
  slice3DManager->setIcon(QIcon(":espina/show_planes.svg"));

  representation.Group     = ExploreToolGroup::SEGMENTATIONS_GROUP;
  representation.Pools    << poolXY << poolXZ << poolYZ << poolVolumetric;
  representation.Managers << sliceManager << slice3DManager << volumetricManager;
  representation.Switches << sliceSwitch << slice3DSwitch << volumetricSwitch;

  return representation;
}

//----------------------------------------------------------------------------
void SegmentationSliceRepresentationFactory::configurePool(RepresentationPoolSPtr           pool,
                                                           ColorEngineSPtr                  colorEngine,
                                                           RepresentationPool::SettingsSPtr settings) const
{
  QObject::connect(colorEngine.get(), SIGNAL(modified()),
                   pool.get(),        SLOT(invalidate()));

  pool->setSettings(settings);
}
