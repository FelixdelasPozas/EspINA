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

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

IntensitySelectionHighlighter SegmentationVolumetricCPUPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationVolumetricCPUPipeline::SegmentationVolumetricCPUPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationVolumetricCPU"}
, m_colorEngine         {colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationVolumetricCPUPipeline::representationState(ConstViewItemAdapterPtr    item,
                                                                           const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationVolumetricCPUPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                                  const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

  ActorList actors;

  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    vtkSmartPointer<vtkImageData> volume = nullptr;
    {
      auto data = readLockVolume(item->output());
      volume    = vtkImage(data, data->bounds());
    }

    auto composite = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

    auto mapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
    mapper->ReleaseDataFlagOn();
    mapper->SetBlendModeToComposite();
    mapper->SetVolumeRayCastFunction(composite);
    mapper->IntermixIntersectingGeometryOn();
    mapper->SetInputData(volume);
    mapper->SetNumberOfThreads(1);
    mapper->Update();

    auto color = s_highlighter.color(m_colorEngine->color(segmentation), item->isSelected());

    auto colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorFunction->AllowDuplicateScalarsOff();
    colorFunction->AddHSVPoint(SEG_BG_VALUE, 0, 0, 0);
    colorFunction->AddHSVPoint(SEG_VOXEL_VALUE, color.hsvHueF(), color.hsvSaturationF(), color.valueF());
    colorFunction->Modified();

    auto piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
    piecewise->AddPoint(SEG_BG_VALUE, 0.0);
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
    actor->UseBoundsOn();
    actor->PickableOn();
    actor->SetProperty(property);
    actor->Update();

    actors << actor;
  }

  return actors;
}

//----------------------------------------------------------------------------
void SegmentationVolumetricCPUPipeline::updateColors(ActorList &actors,
                                                     const ViewItemAdapter *item,
                                                     const RepresentationState &state)
{
  if (actors.size() == 1)
  {
    auto segmentation = segmentationPtr(item);

    auto color = s_highlighter.color(m_colorEngine->color(segmentation), item->isSelected());

    auto actor = dynamic_cast<vtkVolume *>(actors.first().Get());

    auto property = dynamic_cast<vtkVolumeProperty *>(actor->GetProperty());

    auto transferFunction = property->GetRGBTransferFunction();

    transferFunction->AddHSVPoint(SEG_VOXEL_VALUE, color.hsvHueF(), color.hsvSaturationF(), color.valueF());
  }
}

//----------------------------------------------------------------------------
bool SegmentationVolumetricCPUPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
  return true;
}
