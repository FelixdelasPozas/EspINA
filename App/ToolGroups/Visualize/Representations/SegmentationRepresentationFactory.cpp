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
#include <App/ToolGroups/Visualize/Representations/Switches/SegmentationSliceSwitch.h>
#include <App/ToolGroups/Visualize/Representations/Switches/SegmentationContourSwitch.h>
#include <ToolGroups/Visualize/Representations/Switches/SegmentationMeshSwitch.h>
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
#include <GUI/Representations/Settings/SegmentationMeshPoolSettings.h>
#include <Support/Representations/SliceManager.h>
#include <Support/Representations/Slice3DManager.h>
#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/RepresentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;
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
  //createSkeletonRepresentation(representation, context, supportedViews);

  if (supportedViews.testFlag(ESPINA::VIEW_2D))
  {
    createContourRepresentation   (representation, context);
  }
  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    createMeshRepresentation(representation, context);
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

    sliceManager->setName("DisplaySegmentations");
    sliceManager->setIcon(QIcon(":espina/display_segmentations.svg"));
    sliceManager->setDescription(QObject::tr("Display Segmentations"));

    auto sliceSwitch     = std::make_shared<SegmentationSliceSwitch>("DisplaySegmentations", sliceManager, sliceSettings, ViewType::VIEW_2D, timer, context);
    sliceSwitch->setChecked(true);
    sliceSwitch->setShortcut(Qt::Key_Space);

    groupSwitch("1-0", sliceSwitch);

    representation.Managers << sliceManager;
    representation.Switches << sliceSwitch;
  }
  if (supportedViews.testFlag(ESPINA::VIEW_3D))
  {
    auto slice3DManager = std::make_shared<Slice3DManager>(poolSliceXY, poolSliceXZ, poolSliceYZ);

    slice3DManager->setName("DisplaySegmentationProjections");
    slice3DManager->setIcon(QIcon(":espina/display_segmentation_projections.svg"));
    slice3DManager->setDescription(QObject::tr("Display Segmentation Stack Proyections"));

    auto slice3DSwitch   = std::make_shared<SegmentationSliceSwitch>("DisplaySegmentationProjections", slice3DManager, sliceSettings, ViewType::VIEW_3D, timer, context);
    groupSwitch("1-1", slice3DSwitch);

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

  contourManager->setName("DisplaySegmentationContour");
  contourManager->setIcon(QIcon(":espina/display_segmentation_contours.svg"));
  contourManager->setDescription(QObject::tr("Display Segmentation Contours"));

  auto contourSwitch = std::make_shared<SegmentationContourSwitch>(contourManager, contourSettings, ViewType::VIEW_2D, timer, context);
  groupSwitch("1-1", contourSwitch);

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

  auto skeletonSwitch2D     = std::make_shared<BasicRepresentationSwitch>("SkeletonSwitch", skeletonManager2D, ViewType::VIEW_2D, timer, context);

  poolSkeleton3D->setSettings(skeletonSettings);

  skeletonManager3D->setName(QObject::tr("Skeleton 3D Representation"));
  skeletonManager3D->setIcon(QIcon(":espina/tubular.svg"));
  skeletonManager3D->setDescription(QObject::tr("Skeleton 3D Representation"));

  auto skeletonSwitch3D     = std::make_shared<BasicRepresentationSwitch>("Skeleton3DSwitch", skeletonManager3D, ViewType::VIEW_3D, timer, context);

  representation.Pools    << poolSkeleton2DXY << poolSkeleton2DXZ << poolSkeleton2DYZ << poolSkeleton3D;
  representation.Managers << skeletonManager2D << skeletonManager3D;
  representation.Switches << skeletonSwitch2D << skeletonSwitch3D;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::createMeshRepresentation(Representation &representation, Support::Context &context) const
{
  auto scheduler   = context.scheduler();
  auto colorEngine = context.colorEngine();
  auto &timer      = context.timer();

  auto meshesSettings = std::make_shared<SegmentationMeshPoolSettings>();

  auto pipelineMesh   = std::make_shared<SegmentationMeshPipeline>(colorEngine);
  auto poolMesh       = std::make_shared<BasicRepresentationPool>(scheduler, pipelineMesh);
  auto meshManager    = std::make_shared<PassiveActorManager>(poolMesh, ViewType::VIEW_3D, RepresentationManager::EXPORTS_3D);

  auto pipelineSmoothedMesh = std::make_shared<SegmentationSmoothedMeshPipeline>(colorEngine);
  auto poolSmoothedMesh     = std::make_shared<BasicRepresentationPool>(scheduler, pipelineSmoothedMesh);
  auto smoothedMeshManager  = std::make_shared<PassiveActorManager>(poolSmoothedMesh, ViewType::VIEW_3D, RepresentationManager::EXPORTS_3D);

  poolMesh->setSettings(meshesSettings);

  meshManager->setName("DisplaySegmentationMesh");
  meshManager->setIcon(QIcon(":espina/display_segmentations.svg"));
  meshManager->setDescription(QObject::tr("Display Segmentations"));

  poolSmoothedMesh->setSettings(meshesSettings);

  smoothedMeshManager->setName("DisplaySmoothedSegmentationMesh");

  auto meshSwitch = std::make_shared<SegmentationMeshSwitch>(meshManager, smoothedMeshManager, meshesSettings, ViewType::VIEW_3D, timer, context);
  groupSwitch("1-0", meshSwitch);

  representation.Pools    << poolMesh << poolSmoothedMesh;
  representation.Managers << meshManager << smoothedMeshManager;
  representation.Switches << meshSwitch;
}

//----------------------------------------------------------------------------
void SegmentationRepresentationFactory::groupSwitch(const QString &order, ToolSPtr tool) const
{
  tool->setOrder(order,"0-Representations");
}