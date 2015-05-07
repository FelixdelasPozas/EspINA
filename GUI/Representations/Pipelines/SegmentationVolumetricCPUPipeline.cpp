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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Representations/Pipelines/SegmentationVolumetricCPUPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMapper.h>

using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
namespace ESPINA
{
  IntensitySelectionHighlighter SegmentationVolumetricCPUPipeline::s_highlighter;

  //----------------------------------------------------------------------------
  SegmentationVolumetricCPUPipeline::SegmentationVolumetricCPUPipeline(ColorEngineSPtr colorEngine)
  : RepresentationPipeline{"SegmentationVolumetricCPU"}
  , m_colorEngine         {colorEngine}
  {
  }
  
  //----------------------------------------------------------------------------
  RepresentationState SegmentationVolumetricCPUPipeline::representationState(const ViewItemAdapter     *item,
                                                                          const RepresentationState &settings)
  {
    auto segmentation = segmentationPtr(item);

    RepresentationState state;

    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);

    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList SegmentationVolumetricCPUPipeline::createActors(const ViewItemAdapter     *item,
                                                                                 const RepresentationState &state)
  {
    auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

    ActorList actors;

    if (isVisible(state) && hasVolumetricData(segmentation->output()))
    {
      auto data = readLockVolume(item->output());
      auto volume = vtkImage(data, data->bounds());

      auto composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

      auto mapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
      mapper->ReleaseDataFlagOn();
      mapper->SetBlendModeToComposite();
      mapper->SetVolumeRayCastFunction(composite);
      mapper->IntermixIntersectingGeometryOff();
      mapper->SetInputData(volume);
      mapper->SetNumberOfThreads(1);
      mapper->Update();

      auto color = m_colorEngine->color(segmentation);
      double rgba[4], rgb[3], hsv[3];
      s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);
      memcpy(rgb, rgba, 3 * sizeof(double));
      vtkMath::RGBToHSV(rgb, hsv);

      auto colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
      colorFunction->AllowDuplicateScalarsOff();
      colorFunction->AddHSVPoint(SEG_VOXEL_VALUE, hsv[0], hsv[1], hsv[2]);
      colorFunction->Modified();

      auto piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
      piecewise->AddPoint(0, 0.0);
      piecewise->AddPoint(SEG_VOXEL_VALUE, 1.0);
      piecewise->Modified();

      auto property = vtkSmartPointer<vtkVolumeProperty>::New();
      property->SetColor(colorFunction);
      property->SetScalarOpacity(piecewise);
      property->DisableGradientOpacityOff();
      property->SetSpecular(0.5);
      property->ShadeOn();
      property->SetInterpolationTypeToLinear();
      property->Modified();

      auto actor = vtkSmartPointer<vtkVolume>::New();
      actor->SetMapper(mapper);
      actor->SetProperty(property);
      actor->Update();

      actors << actor;
    }

    return actors;
  }

  //----------------------------------------------------------------------------
  bool SegmentationVolumetricCPUPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
    return true;
  }

} // namespace ESPINA
