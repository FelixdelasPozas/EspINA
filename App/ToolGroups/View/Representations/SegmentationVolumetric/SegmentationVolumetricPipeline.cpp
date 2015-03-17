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
#include <ToolGroups/View/Representations/SegmentationVolumetric/SegmentationVolumetricPipeline.h>
#include <ToolGroups/View/Representations/RepresentationSettings.h>
#include <GUI/Representations/SegmentationPipeline.h>

// VTK
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMapper.h>

namespace ESPINA
{
  TransparencySelectionHighlighter SegmentationVolumetricPipeline::s_highlighter;

  //----------------------------------------------------------------------------
  SegmentationVolumetricPipeline::SegmentationVolumetricPipeline(ColorEngineSPtr colorEngine)
  : RepresentationPipeline("SegmentationVolumetric")
  , m_colorEngine{colorEngine}
  {
  }
  
  //----------------------------------------------------------------------------
  RepresentationState SegmentationVolumetricPipeline::representationState(const ViewItemAdapter     *item,
                                                                          const RepresentationState &settings)
  {
    auto segmentation = segmentationPtr(item);

    RepresentationState state;

    state.apply(SegmentationPipeline::Settings(segmentation));
    state.apply(settings);

    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList SegmentationVolumetricPipeline::createActors(const ViewItemAdapter     *item,
                                                                                 const RepresentationState &state)
  {
    auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

    ActorList actors;

    if (isVisible(state) && hasVolumetricData(segmentation->output()))
    {
      auto volume = volumetricData(segmentation->output());
      auto vtkData = vtkImage(volume, volume->bounds());

      auto composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
      auto mapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
      mapper->ReleaseDataFlagOn();
      mapper->SetBlendModeToComposite();
      mapper->SetVolumeRayCastFunction(composite);
      mapper->IntermixIntersectingGeometryOff();
      mapper->SetInputData(vtkData);
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
      property->Modified();

      auto actor = vtkSmartPointer<vtkVolume>::New();
      actor->SetMapper(mapper);
      actor->SetProperty(property);
      actor->Update();

      actors << actor;
    }

    qDebug() << "return" << actors.size();

    return actors;
  }

  //----------------------------------------------------------------------------
  bool SegmentationVolumetricPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    auto segmentation = segmentationPtr(item);

    if(!hasVolumetricData(segmentation->output())) return false;

    auto volume = volumetricData(segmentation->output());
    auto pointVolume = volume->itkImage(Bounds(point));

    auto pixel = reinterpret_cast<unsigned char *>(volume->itkImage(Bounds(point))->GetBufferPointer());

    return (*pixel == SEG_VOXEL_VALUE);
  }

} // namespace ESPINA
