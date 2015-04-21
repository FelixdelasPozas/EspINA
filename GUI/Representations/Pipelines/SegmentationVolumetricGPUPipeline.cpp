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
#include <GUI/Representations/Pipelines/SegmentationVolumetricGPUPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>

// VTK
#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastCompositeFunction.h>

namespace ESPINA
{
  TransparencySelectionHighlighter SegmentationVolumetricGPUPipeline::s_highlighter;

  //----------------------------------------------------------------------------
  SegmentationVolumetricGPUPipeline::SegmentationVolumetricGPUPipeline(ColorEngineSPtr colorEngine)
  : RepresentationPipeline("SegmentationVolumetricGPU")
  , m_colorEngine{colorEngine}
  {
  }
  
  //----------------------------------------------------------------------------
  RepresentationState SegmentationVolumetricGPUPipeline::representationState(const ViewItemAdapter     *item,
                                                                             const RepresentationState &settings)
  {
    auto segmentation = segmentationPtr(item);

    RepresentationState state;

    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);

    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList SegmentationVolumetricGPUPipeline::createActors(const ViewItemAdapter     *item,
                                                                                    const RepresentationState &state)
  {
    auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

    ActorList actors;

    if (isVisible(state) && hasVolumetricData(segmentation->output()))
    {
      auto data = volumetricData(segmentation->output());
      auto volume = vtkImage(data, data->bounds());

      auto mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
      mapper->ReleaseDataFlagOn();
      mapper->GlobalWarningDisplayOff();
      mapper->AutoAdjustSampleDistancesOn();
      mapper->SetScalarModeToUsePointData();
      mapper->SetBlendModeToComposite();
      mapper->SetMaxMemoryFraction(1);
      mapper->SetInputData(volume);
      mapper->Update();

      auto color = m_colorEngine->color(segmentation);
      double rgba[4], rgb[3], hsv[3];
      s_highlighter.lut(color)->GetTableValue(1,rgba);
      memcpy(rgb, rgba, 3 * sizeof(double));
      vtkMath::RGBToHSV(rgb, hsv);

      auto colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
      colorFunction->AllowDuplicateScalarsOff();
      colorFunction->AddHSVPoint(255, hsv[0], hsv[1], hsv[2]);
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
      property->IndependentComponentsOn();
      property->Modified();

      auto actor = vtkSmartPointer<vtkVolume>::New();
      actor->UseBoundsOn();
      actor->PickableOn();
      actor->SetMapper(mapper);
      actor->SetProperty(property);
      actor->Update();

      actors << actor;
    }

    return actors;
  }

  //----------------------------------------------------------------------------
  bool SegmentationVolumetricGPUPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    Q_ASSERT(hasVolumetricData(item->output()));
    auto volume = volumetricData(item->output());
    return isSegmentationVoxel(volume, point);
  }

} // namespace ESPINA
