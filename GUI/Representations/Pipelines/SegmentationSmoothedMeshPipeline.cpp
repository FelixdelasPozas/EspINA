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

// VTK
#include <vtkActor.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>

using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

namespace ESPINA
{
  IntensitySelectionHighlighter SegmentationSmoothedMeshPipeline::s_highlighter;

  //----------------------------------------------------------------------------
  SegmentationSmoothedMeshPipeline::SegmentationSmoothedMeshPipeline(ColorEngineSPtr colorEngine)
  : RepresentationPipeline{"SegmentationSmoothedMesh"}
  , m_colorEngine         {colorEngine}
  {
  }

  //----------------------------------------------------------------------------
  RepresentationState SegmentationSmoothedMeshPipeline::representationState(const ViewItemAdapter     *item,
                                                                            const RepresentationState &settings)
  {
    auto segmentation = segmentationPtr(item);

    RepresentationState state;

    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);

    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList SegmentationSmoothedMeshPipeline::createActors(const ViewItemAdapter     *item,
                                                                                   const RepresentationState &state)
  {
    ActorList actors;
    
    auto segmentation = segmentationPtr(item);

    if(isVisible(state) && hasMeshData(segmentation->output()))
    {
      auto data = meshData(segmentation->output());

      auto decimate = vtkSmartPointer<vtkDecimatePro>::New();
      decimate->ReleaseDataFlagOn();
      decimate->SetGlobalWarningDisplay(false);
      decimate->SetTargetReduction(0.95);
      decimate->PreserveTopologyOn();
      decimate->SplittingOff();
      decimate->SetInputData(data->mesh());

      auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
      smoother->ReleaseDataFlagOn();
      smoother->SetGlobalWarningDisplay(false);
      smoother->BoundarySmoothingOn();
      smoother->FeatureEdgeSmoothingOn();
      smoother->SetNumberOfIterations(15);
      smoother->SetFeatureAngle(120);
      smoother->SetEdgeAngle(90);
      smoother->SetInputConnection(decimate->GetOutputPort());

      auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
      normals->ReleaseDataFlagOn();
      normals->SetFeatureAngle(120);
      normals->SetInputConnection(smoother->GetOutputPort());

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->ReleaseDataFlagOn();
      mapper->ImmediateModeRenderingOn();
      mapper->ScalarVisibilityOff();
      mapper->SetInputConnection(normals->GetOutputPort());
      mapper->Update();

      auto color = m_colorEngine->color(segmentation);
      double rgba[4];
      s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);
      actor->GetProperty()->SetSpecular(0.2);
      actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
      actor->GetProperty()->SetOpacity(1);
      actor->Modified();

      actors << actor;
    }

    return actors;
  }
  
  //----------------------------------------------------------------------------
  bool SegmentationSmoothedMeshPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
    return true;
  }

} // namespace ESPINA
