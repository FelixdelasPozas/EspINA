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
#include <App/ToolGroups/Visualize/Representations/SegmentationRepresentationFactory.h>
#include <App/ToolGroups/Visualize/VisualizeToolGroup.h>
#include <GUI/Representations/Pools/BufferedRepresentationPool.h>
#include <GUI/Representations/Pipelines/SegmentationContourPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationMeshPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton2DPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSlicePipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSmoothedMeshPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationVolumetricCPUPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationVolumetricGPUPipeline.h>
#include <GUI/Representations/Pools/BasicRepresentationPool.h>
#include <GUI/Representations/Settings/SegmentationContourPoolSettings.h>
#include <GUI/Representations/Settings/SegmentationSlicePoolSettings.h>
#include <GUI/Representations/ReadyActorManager.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/RepresentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::Support::Representations::Utils;

//----------------------------------------------------------------------------
SegmentationRepresentationFactory::SegmentationRepresentationFactory(SchedulerSPtr scheduler)
: m_scheduler(scheduler)
{
}

//----------------------------------------------------------------------------
Representation ESPINA::SegmentationRepresentationFactory::createRepresentation(ColorEngineSPtr colorEngine) const
{
  const unsigned WINDOW_SIZE = 5;

  Representation representation;

  representation.Group = SEGMENTATIONS_GROUP;

  createSliceRepresentation(representation, colorEngine, WINDOW_SIZE);
  createContourRepresentation(representation, colorEngine, WINDOW_SIZE);
  createSkeletonRepresentation(representation, colorEngine, WINDOW_SIZE);
  createVolumetricRepresentation(representation, colorEngine);
  createMeshRepresentation(representation, colorEngine);

  return representation;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::configurePool(RepresentationPoolSPtr           pool,
                                                      ColorEngineSPtr                  colorEngine,
                                                      RepresentationPool::SettingsSPtr settings) const
{
  QObject::connect(colorEngine.get(), SIGNAL(modified()),
                   pool.get(),        SLOT(invalidate()));

  pool->setSettings(settings);
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createSliceRepresentation(Representation &representation, ColorEngineSPtr colorEngine, const unsigned int windowSize) const
{
  auto sliceSettings   = std::make_shared<SegmentationSlicePoolSettings>();
  auto pipelineSliceXY = std::make_shared<SegmentationSlicePipeline>(Plane::XY, colorEngine);
  auto pipelineSliceXZ = std::make_shared<SegmentationSlicePipeline>(Plane::XZ, colorEngine);
  auto pipelineSliceYZ = std::make_shared<SegmentationSlicePipeline>(Plane::YZ, colorEngine);
  auto poolSliceXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineSliceXY, m_scheduler, windowSize);
  auto poolSliceXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineSliceXZ, m_scheduler, windowSize);
  auto poolSliceYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineSliceYZ, m_scheduler, windowSize);
  auto sliceManager    = std::make_shared<SliceManager>(poolSliceXY, poolSliceXZ, poolSliceYZ);
  auto sliceSwitch     = std::make_shared<BasicRepresentationSwitch>(sliceManager, ViewType::VIEW_2D);
  auto slice3DManager  = std::make_shared<Slice3DManager>(poolSliceXY, poolSliceXZ, poolSliceYZ);
  auto slice3DSwitch   = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D);

  configurePool(poolSliceXY, colorEngine, sliceSettings);
  configurePool(poolSliceXZ, colorEngine, sliceSettings);
  configurePool(poolSliceYZ, colorEngine, sliceSettings);

  sliceManager->setName(QObject::tr("Slice Representation"));
  sliceManager->setIcon(QIcon(":espina/segmentations_slice_switch.svg"));

  sliceSwitch->setActive(true);

  slice3DManager->setName(QObject::tr("Slice Representation"));
  slice3DManager->setIcon(QIcon(":espina/show_planes.svg"));

  representation.Pools    << poolSliceXY << poolSliceXZ << poolSliceYZ;
  representation.Managers << sliceManager << slice3DManager;
  representation.Switches << sliceSwitch << slice3DSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createContourRepresentation(Representation &representation, ColorEngineSPtr colorEngine, const unsigned int windowSize) const
{
  auto contourSettings   = std::make_shared<SegmentationContourPoolSettings>();
  auto pipelineContourXY = std::make_shared<SegmentationContourPipeline>(Plane::XY, colorEngine);
  auto pipelineContourXZ = std::make_shared<SegmentationContourPipeline>(Plane::XZ, colorEngine);
  auto pipelineContourYZ = std::make_shared<SegmentationContourPipeline>(Plane::YZ, colorEngine);
  auto poolContourXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineContourXY, m_scheduler, windowSize);
  auto poolContourXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineContourXZ, m_scheduler, windowSize);
  auto poolContourYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineContourYZ, m_scheduler, windowSize);
  auto contourManager    = std::make_shared<SliceManager>(poolContourXY, poolContourXZ, poolContourYZ);
  auto contourSwitch     = std::make_shared<BasicRepresentationSwitch>(contourManager, ViewType::VIEW_2D);

  configurePool(poolContourXY, colorEngine, contourSettings);
  configurePool(poolContourXZ, colorEngine, contourSettings);
  configurePool(poolContourYZ, colorEngine, contourSettings);

  contourManager->setName(QObject::tr("Contour Representation"));
  contourManager->setIcon(QIcon(":espina/contour.png"));

  representation.Pools    << poolContourXY << poolContourXZ << poolContourYZ;
  representation.Managers << contourManager;
  representation.Switches << contourSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createSkeletonRepresentation(Representation &representation, ColorEngineSPtr colorEngine, const unsigned int windowSize) const
{
  auto skeletonSettings     = std::make_shared<RepresentationPool::Settings>();
  auto pipeline2DSkeletonXY = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::XY, colorEngine);
  auto pipeline2DSkeletonXZ = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::XZ, colorEngine);
  auto pipeline2DSkeletonYZ = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::YZ, colorEngine);
  auto poolSkeleton2DXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipeline2DSkeletonXY, m_scheduler, windowSize);
  auto poolSkeleton2DXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipeline2DSkeletonXZ, m_scheduler, windowSize);
  auto poolSkeleton2DYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipeline2DSkeletonYZ, m_scheduler, windowSize);
  auto skeletonManager2D    = std::make_shared<SliceManager>(poolSkeleton2DXY, poolSkeleton2DXZ, poolSkeleton2DYZ);
  auto skeletonSwitch2D     = std::make_shared<BasicRepresentationSwitch>(skeletonManager2D, ViewType::VIEW_2D);

  auto pipelineSkeleton3D   = std::make_shared<SegmentationSkeleton3DPipeline>(colorEngine);
  auto poolSkeleton3D       = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineSkeleton3D);
  auto skeletonManager3D    = std::make_shared<ReadyActorManager>(poolSkeleton3D, ViewType::VIEW_3D);
  auto skeletonSwitch3D     = std::make_shared<BasicRepresentationSwitch>(skeletonManager3D, ViewType::VIEW_3D);

  configurePool(poolSkeleton2DXY, colorEngine, skeletonSettings);
  configurePool(poolSkeleton2DXZ, colorEngine, skeletonSettings);
  configurePool(poolSkeleton2DYZ, colorEngine, skeletonSettings);

  skeletonManager2D->setName(QObject::tr("Skeleton 2D Representation"));
  skeletonManager2D->setIcon(QIcon(":espina/tubular.svg"));

  configurePool(poolSkeleton3D, colorEngine, skeletonSettings);

  skeletonManager3D->setName(QObject::tr("Skeleton 3D Representation"));
  skeletonManager3D->setIcon(QIcon(":espina/tubular.svg"));

  representation.Pools    << poolSkeleton2DXY << poolSkeleton2DXZ << poolSkeleton2DYZ << poolSkeleton3D;
  representation.Managers << skeletonManager2D << skeletonManager3D;
  representation.Switches << skeletonSwitch2D << skeletonSwitch3D;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createVolumetricRepresentation(Representation &representation, ColorEngineSPtr colorEngine) const
{
  auto volumetricSettings   = std::make_shared<RepresentationPool::Settings>();
  auto pipelineVolumeCPU    = std::make_shared<SegmentationVolumetricCPUPipeline>(colorEngine);
  auto poolVolumetricCPU    = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineVolumeCPU);
  auto volumetricCPUManager = std::make_shared<ReadyActorManager>(poolVolumetricCPU, ViewType::VIEW_3D);
  auto volumetricCPUSwitch  = std::make_shared<BasicRepresentationSwitch>(volumetricCPUManager, ViewType::VIEW_3D);

  auto pipelineVolumeGPU    = std::make_shared<SegmentationVolumetricGPUPipeline>(colorEngine);
  auto poolVolumetricGPU    = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineVolumeGPU);
  auto volumetricGPUManager = std::make_shared<ReadyActorManager>(poolVolumetricGPU, ViewType::VIEW_3D);
  auto volumetricGPUSwitch  = std::make_shared<BasicRepresentationSwitch>(volumetricGPUManager, ViewType::VIEW_3D);

  configurePool(poolVolumetricCPU, colorEngine, volumetricSettings);
  
  volumetricCPUManager->setName(QObject::tr("Volumetric Representation"));
  volumetricCPUManager->setIcon(QIcon(":espina/voxel.png"));

  configurePool(poolVolumetricGPU, colorEngine, volumetricSettings);

  volumetricGPUManager->setName(QObject::tr("Volumetric GPU Representation"));
  volumetricGPUManager->setIcon(QIcon(":espina/voxelGPU.png"));

  representation.Pools    << poolVolumetricCPU << poolVolumetricGPU;
  representation.Managers << volumetricCPUManager << volumetricGPUManager;
  representation.Switches << volumetricCPUSwitch << volumetricGPUSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createMeshRepresentation(Representation &representation, ColorEngineSPtr colorEngine) const
{
  auto meshesSettings = std::make_shared<RepresentationPool::Settings>();
  auto pipelineMesh   = std::make_shared<SegmentationMeshPipeline>(colorEngine);
  auto poolMesh       = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineMesh);
  auto meshManager    = std::make_shared<ReadyActorManager>(poolMesh, ViewType::VIEW_3D);
  auto meshSwitch     = std::make_shared<BasicRepresentationSwitch>(meshManager, ViewType::VIEW_3D);

  auto pipelineSmoothedMesh = std::make_shared<SegmentationSmoothedMeshPipeline>(colorEngine);
  auto poolSmoothedMesh     = std::make_shared<BasicRepresentationPool>(m_scheduler, pipelineSmoothedMesh);
  auto smoothedMeshManager  = std::make_shared<ReadyActorManager>(poolSmoothedMesh, ViewType::VIEW_3D);
  auto smoothedMeshSwitch   = std::make_shared<BasicRepresentationSwitch>(smoothedMeshManager, ViewType::VIEW_3D);

  configurePool(poolMesh, colorEngine, meshesSettings);

  meshManager->setName(QObject::tr("Mesh Representation"));
  meshManager->setIcon(QIcon(":espina/mesh.png"));

  configurePool(poolSmoothedMesh, colorEngine, meshesSettings);

  smoothedMeshManager->setName(QObject::tr("Smoothed Mesh Representation"));
  smoothedMeshManager->setIcon(QIcon(":espina/smoothedmesh.png"));

  representation.Pools    << poolMesh << poolSmoothedMesh;
  representation.Managers << meshManager << smoothedMeshManager;
  representation.Switches << meshSwitch << smoothedMeshSwitch;
}
