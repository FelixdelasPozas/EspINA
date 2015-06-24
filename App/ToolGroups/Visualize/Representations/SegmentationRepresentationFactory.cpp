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
#include <GUI/Representations/Managers/PassiveActorManager.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/RepresentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::Support::Representations::Utils;
using ESPINA::GUI::Representations::Managers::PassiveActorManager;
using ESPINA::GUI::Representations::RepresentationManager;

const unsigned SegmentationRepresentationFactory::WINDOW_SIZE = 5;

//----------------------------------------------------------------------------
SegmentationRepresentationFactory::SegmentationRepresentationFactory()
{
}

//----------------------------------------------------------------------------
Representation SegmentationRepresentationFactory::doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const
{
  Representation representation;

  representation.Group = SEGMENTATIONS_GROUP;

  createSliceRepresentation(representation, context, supportedViews);

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    createContourRepresentation   (representation, context);
  }
  //createSkeletonRepresentation  (representation, context);
  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    createVolumetricRepresentation(representation, context);
    createMeshRepresentation      (representation, context);
  }

  return representation;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createSliceRepresentation(Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto sliceSettings   = std::make_shared<SegmentationSlicePoolSettings>();
  auto pipelineSliceXY = std::make_shared<SegmentationSlicePipeline>(Plane::XY, colorEngine);
  auto pipelineSliceXZ = std::make_shared<SegmentationSlicePipeline>(Plane::XZ, colorEngine);
  auto pipelineSliceYZ = std::make_shared<SegmentationSlicePipeline>(Plane::YZ, colorEngine);
  auto poolSliceXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineSliceXY, scheduler, WINDOW_SIZE);
  auto poolSliceXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineSliceXZ, scheduler, WINDOW_SIZE);
  auto poolSliceYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineSliceYZ, scheduler, WINDOW_SIZE);

  poolSliceXY->setSettings(sliceSettings);
  poolSliceXZ->setSettings(sliceSettings);
  poolSliceYZ->setSettings(sliceSettings);

  representation.Pools << poolSliceXY << poolSliceXZ << poolSliceYZ;

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    auto sliceManager    = std::make_shared<SliceManager>(poolSliceXY, poolSliceXZ, poolSliceYZ);

    sliceManager->setName(QObject::tr("Segmentation Slice Representation"));
    sliceManager->setIcon(QIcon(":espina/segmentations_slice_switch.svg"));
    sliceManager->setDescription(QObject::tr("Segmentation Slice Representation"));

    auto sliceSwitch     = std::make_shared<SegmentationRepresentationSwitch>(sliceManager, sliceSettings, ViewType::VIEW_2D, timer, context);
    sliceSwitch->setChecked(true);
    groupSwitch(sliceSwitch);

    representation.Managers << sliceManager;
    representation.Switches << sliceSwitch;
  }
  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    auto slice3DManager  = std::make_shared<Slice3DManager>(poolSliceXY, poolSliceXZ, poolSliceYZ);

    slice3DManager->setName(QObject::tr("Slice Representation"));
    slice3DManager->setIcon(QIcon(":espina/segmentations_slice3D_switch.svg"));
    slice3DManager->setDescription(QObject::tr("Segmentation 3D Slice Representation"));

    auto slice3DSwitch   = std::make_shared<BasicRepresentationSwitch>(slice3DManager, ViewType::VIEW_3D, timer, context);
    groupSwitch(slice3DSwitch);

    representation.Managers << slice3DManager;
    representation.Switches << slice3DSwitch;
  }
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createContourRepresentation(Representation &representation, Support::Context &context) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto contourSettings   = std::make_shared<SegmentationContourPoolSettings>();
  auto pipelineContourXY = std::make_shared<SegmentationContourPipeline>(Plane::XY, colorEngine);
  auto pipelineContourXZ = std::make_shared<SegmentationContourPipeline>(Plane::XZ, colorEngine);
  auto pipelineContourYZ = std::make_shared<SegmentationContourPipeline>(Plane::YZ, colorEngine);
  auto poolContourXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipelineContourXY, scheduler, WINDOW_SIZE);
  auto poolContourXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipelineContourXZ, scheduler, WINDOW_SIZE);
  auto poolContourYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipelineContourYZ, scheduler, WINDOW_SIZE);
  auto contourManager    = std::make_shared<SliceManager>(poolContourXY, poolContourXZ, poolContourYZ);

  poolContourXY->setSettings(contourSettings);
  poolContourXZ->setSettings(contourSettings);
  poolContourYZ->setSettings(contourSettings);

  contourManager->setName(QObject::tr("Contour Representation"));
  contourManager->setIcon(QIcon(":espina/contour.png"));
  contourManager->setDescription(QObject::tr("Segmentation Contour Representation"));

  auto contourSwitch = std::make_shared<BasicRepresentationSwitch>(contourManager, ViewType::VIEW_2D, timer, context);
  groupSwitch(contourSwitch);

  representation.Pools    << poolContourXY << poolContourXZ << poolContourYZ;
  representation.Managers << contourManager;
  representation.Switches << contourSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createSkeletonRepresentation(Representation &representation, Support::Context &context) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto skeletonSettings     = std::make_shared<SegmentationSlicePoolSettings>(); // TODO: create SkeletonPoolSettings
  auto pipeline2DSkeletonXY = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::XY, colorEngine);
  auto pipeline2DSkeletonXZ = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::XZ, colorEngine);
  auto pipeline2DSkeletonYZ = std::make_shared<SegmentationSkeleton2DPipeline>(Plane::YZ, colorEngine);
  auto poolSkeleton2DXY     = std::make_shared<BufferedRepresentationPool>(Plane::XY, pipeline2DSkeletonXY, scheduler, WINDOW_SIZE);
  auto poolSkeleton2DXZ     = std::make_shared<BufferedRepresentationPool>(Plane::XZ, pipeline2DSkeletonXZ, scheduler, WINDOW_SIZE);
  auto poolSkeleton2DYZ     = std::make_shared<BufferedRepresentationPool>(Plane::YZ, pipeline2DSkeletonYZ, scheduler, WINDOW_SIZE);
  auto skeletonManager2D    = std::make_shared<SliceManager>(poolSkeleton2DXY, poolSkeleton2DXZ, poolSkeleton2DYZ);

  auto pipelineSkeleton3D   = std::make_shared<SegmentationSkeleton3DPipeline>(colorEngine);
  auto poolSkeleton3D       = std::make_shared<BasicRepresentationPool>(scheduler, pipelineSkeleton3D);
  auto skeletonManager3D    = std::make_shared<PassiveActorManager>(poolSkeleton3D, ViewType::VIEW_3D);

  poolSkeleton2DXY->setSettings(skeletonSettings);
  poolSkeleton2DXZ->setSettings(skeletonSettings);
  poolSkeleton2DYZ->setSettings(skeletonSettings);

  skeletonManager2D->setName(QObject::tr("Skeleton 2D Representation"));
  skeletonManager2D->setIcon(QIcon(":espina/tubular.svg"));
  skeletonManager2D->setDescription(QObject::tr("Skeleton 2D Representation"));

  auto skeletonSwitch2D     = std::make_shared<BasicRepresentationSwitch>(skeletonManager2D, ViewType::VIEW_2D, timer, context);

  poolSkeleton3D->setSettings(skeletonSettings);

  skeletonManager3D->setName(QObject::tr("Skeleton 3D Representation"));
  skeletonManager3D->setIcon(QIcon(":espina/tubular.svg"));
  skeletonManager3D->setDescription(QObject::tr("Skeleton 3D Representation"));

  auto skeletonSwitch3D     = std::make_shared<BasicRepresentationSwitch>(skeletonManager3D, ViewType::VIEW_3D, timer, context);

  representation.Pools    << poolSkeleton2DXY << poolSkeleton2DXZ << poolSkeleton2DYZ << poolSkeleton3D;
  representation.Managers << skeletonManager2D << skeletonManager3D;
  representation.Switches << skeletonSwitch2D << skeletonSwitch3D;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createVolumetricRepresentation(Representation &representation, Support::Context &context) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto volumetricSettings   = std::make_shared<PoolSettings>();
  auto pipelineVolumeCPU    = std::make_shared<SegmentationVolumetricCPUPipeline>(colorEngine);
  auto poolVolumetricCPU    = std::make_shared<BasicRepresentationPool>(scheduler, pipelineVolumeCPU);
  auto volumetricCPUManager = std::make_shared<PassiveActorManager>(poolVolumetricCPU, ViewType::VIEW_3D);

  auto pipelineVolumeGPU    = std::make_shared<SegmentationVolumetricGPUPipeline>(colorEngine);
  auto poolVolumetricGPU    = std::make_shared<BasicRepresentationPool>(scheduler, pipelineVolumeGPU);
  auto volumetricGPUManager = std::make_shared<PassiveActorManager>(poolVolumetricGPU, ViewType::VIEW_3D);

  poolVolumetricCPU->setSettings(volumetricSettings);
  
  volumetricCPUManager->setName(QObject::tr("Volumetric Representation"));
  volumetricCPUManager->setIcon(QIcon(":espina/voxel.png"));
  volumetricCPUManager->setDescription(QObject::tr("Segmentation Volumetric Representation"));

  poolVolumetricGPU->setSettings(volumetricSettings);

  auto volumetricCPUSwitch  = std::make_shared<BasicRepresentationSwitch>(volumetricCPUManager, ViewType::VIEW_3D, timer, context);
  groupSwitch(volumetricCPUSwitch);

  volumetricGPUManager->setName(QObject::tr("Volumetric GPU Representation"));
  volumetricGPUManager->setIcon(QIcon(":espina/voxelGPU.png"));
  volumetricGPUManager->setDescription(QObject::tr("Segmentation Volumetric Representation By GPU"));

  auto volumetricGPUSwitch  = std::make_shared<BasicRepresentationSwitch>(volumetricGPUManager, ViewType::VIEW_3D, timer, context);
  groupSwitch(volumetricGPUSwitch);

  representation.Pools    << poolVolumetricCPU << poolVolumetricGPU;
  representation.Managers << volumetricCPUManager << volumetricGPUManager;
  representation.Switches << volumetricCPUSwitch << volumetricGPUSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createMeshRepresentation(Representation &representation, Support::Context &context) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto meshesSettings = std::make_shared<PoolSettings>();
  auto pipelineMesh   = std::make_shared<SegmentationMeshPipeline>(colorEngine);
  auto poolMesh       = std::make_shared<BasicRepresentationPool>(scheduler, pipelineMesh);
  auto meshManager    = std::make_shared<PassiveActorManager>(poolMesh, ViewType::VIEW_3D, RepresentationManager::EXPORTS_3D);

  auto pipelineSmoothedMesh = std::make_shared<SegmentationSmoothedMeshPipeline>(colorEngine);
  auto poolSmoothedMesh     = std::make_shared<BasicRepresentationPool>(scheduler, pipelineSmoothedMesh);
  auto smoothedMeshManager  = std::make_shared<PassiveActorManager>(poolSmoothedMesh, ViewType::VIEW_3D, RepresentationManager::EXPORTS_3D);

  poolMesh->setSettings(meshesSettings);

  meshManager->setName(QObject::tr("Mesh Representation"));
  meshManager->setIcon(QIcon(":espina/mesh.png"));
  meshManager->setDescription(QObject::tr("Mesh Representation"));

  auto meshSwitch     = std::make_shared<BasicRepresentationSwitch>(meshManager, ViewType::VIEW_3D, timer, context);
  groupSwitch(meshSwitch);

  poolSmoothedMesh->setSettings(meshesSettings);

  smoothedMeshManager->setName(QObject::tr("Smoothed Mesh Representation"));
  smoothedMeshManager->setIcon(QIcon(":espina/smoothedmesh.png"));
  smoothedMeshManager->setDescription(QObject::tr("Smoothed Mesh Representation"));

  auto smoothedMeshSwitch   = std::make_shared<BasicRepresentationSwitch>(smoothedMeshManager, ViewType::VIEW_3D, timer, context);
  groupSwitch(smoothedMeshSwitch);

  representation.Pools    << poolMesh << poolSmoothedMesh;
  representation.Managers << meshManager << smoothedMeshManager;
  representation.Switches << meshSwitch << smoothedMeshSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::groupSwitch(ToolSPtr tool) const
{
  tool->setGroupWith("2_segmentation_reps");
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::groupSwitch3D(ToolSPtr tool) const
{
  tool->setGroupWith("2_segmentation_reps_3D");
}

//----------------------------------------------------------------------------
SegmentationRepresentationSwitch::SegmentationRepresentationSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                                                                   std::shared_ptr<SegmentationSlicePoolSettings> settings,
                                                                   ViewTypeFlags supportedViews,
                                                                   Timer& timer,
                                                                   Support::Context& context)
: BasicRepresentationSwitch(manager, supportedViews, timer, context)
, m_settings{settings}
{
  initWidgets();
}

//----------------------------------------------------------------------------
void SegmentationRepresentationSwitch::onOpacityChanged(int value)
{
  // TODO: increment timer, invalidate representations and propagate to manager?

  m_settings->setOpacity(static_cast<double>(value/100.0));
}

//----------------------------------------------------------------------------
void SegmentationRepresentationSwitch::initWidgets()
{
  m_opacityWidget = new GUI::Widgets::NumericalInput();
  m_opacityWidget->setLabelText(tr("Opacity"));

  addSettingsWidget(m_opacityWidget);

  m_opacityWidget->setMinimum(1);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setValue(m_settings->opacity()*100);
  m_opacityWidget->setSpinBoxVisibility(false);
  m_opacityWidget->setLabelText(tr("Opacity"));

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(onOpacityChanged(int)));
}
