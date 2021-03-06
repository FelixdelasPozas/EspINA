/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Representations/Pipelines/SegmentationSmoothedMeshPipeline.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationMeshPoolSettings.h>

// VTK
#include <vtkActor.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

IntensitySelectionHighlighter SegmentationSmoothedMeshPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationSmoothedMeshPipeline::SegmentationSmoothedMeshPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationSmoothedMesh"}
, m_colorEngine         {colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationSmoothedMeshPipeline::representationState(ConstViewItemAdapterPtr  item,
                                                                          const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSmoothedMeshPipeline::createActors(ConstViewItemAdapterPtr     item,
                                                                                 const RepresentationState &state)
{
  ActorList actors;

  auto segmentation = segmentationPtr(item);

  if(isVisible(state) && hasMeshData(segmentation->output()))
  {
    auto smoothValue = state.getValue<int>(SegmentationMeshPoolSettings::SMOOTH_KEY);
    auto ratio       = smoothValue/100.0;
    auto iterations  = static_cast<int>(15*(ratio+1.0));

    VolumeBounds meshBounds;
    vtkSmartPointer<vtkPolyData> mesh = nullptr;
    {
      auto data  = readLockMesh(segmentation->output(), DataUpdatePolicy::Ignore);
      meshBounds = data->bounds();
      mesh       = data->mesh();
    }

    auto decimate = vtkSmartPointer<vtkDecimatePro>::New();
    decimate->ReleaseDataFlagOn();
    decimate->SetGlobalWarningDisplay(false);
    decimate->SetTargetReduction(ratio);
    decimate->PreserveTopologyOn();
    decimate->SplittingOff();
    decimate->SetInputData(mesh);

    auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->ReleaseDataFlagOn();
    smoother->SetGlobalWarningDisplay(false);
    smoother->BoundarySmoothingOn();
    smoother->FeatureEdgeSmoothingOn();
    smoother->SetNumberOfIterations(iterations);
    smoother->SetFeatureAngle(120);
    smoother->SetEdgeAngle(90);
    smoother->SetInputConnection(decimate->GetOutputPort());

    auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->ReleaseDataFlagOn();
    normals->SetFeatureAngle(120);
    normals->SetInputConnection(smoother->GetOutputPort());

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->ReleaseDataFlagOn();
    mapper->ScalarVisibilityOff();
    mapper->SetInputConnection(normals->GetOutputPort());
    mapper->Update();

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetSpecular(0.2);
    actor->GetProperty()->SetOpacity(1);
    actor->Modified();

    // the smoothing process produces a shift in place and a reduction of the
    // actor towards the centroid, we'll try to fix it.
    auto center = centroid(meshBounds);
    double position[3];
    double bounds[6];
    actor->GetBounds(bounds);
    actor->GetPosition(position);
    for(int i = 0; i < 3; ++i)
    {
      position[i] -= ((bounds[2*i]+bounds[(2*i)+1])/2.0) - center[i];
    }
    actor->SetPosition(position);

    actors << actor;
  }

  return actors;
}

//----------------------------------------------------------------------------
void SegmentationSmoothedMeshPipeline::updateColors(RepresentationPipeline::ActorList &actors,
                                                    ConstViewItemAdapterPtr           item,
                                                    const RepresentationState         &state)
{
  if (actors.size() == 1)
  {
    auto segmentation = segmentationPtr(item);

    QColor color;
    if(segmentation->colorEngine())
    {
      color = segmentation->colorEngine()->color(segmentation);
    }
    else
    {
      color = m_colorEngine->color(segmentation);
    }

    color = s_highlighter.color(color, item->isSelected());

    auto actor = dynamic_cast<vtkActor *>(actors.first().Get());

    actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
  }
}

//----------------------------------------------------------------------------
bool SegmentationSmoothedMeshPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
  return true;
}
