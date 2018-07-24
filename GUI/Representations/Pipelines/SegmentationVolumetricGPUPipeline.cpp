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
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkSmartVolumeMapper.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

IntensitySelectionHighlighter SegmentationVolumetricGPUPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationVolumetricGPUPipeline::SegmentationVolumetricGPUPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline("SegmentationVolumetricGPU")
, m_colorEngine{colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationVolumetricGPUPipeline::representationState(ConstViewItemAdapterPtr    item,
                                                                           const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationVolumetricGPUPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                                  const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

  ActorList actors;

  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    vtkSmartPointer<vtkImageData> volume = nullptr;
    {
      auto data = readLockVolume(segmentation->output());
      volume    = vtkImage(data, data->bounds());
    }

    auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    mapper->ReleaseDataFlagOn();
    mapper->GlobalWarningDisplayOff();
    mapper->DebugOff();
    mapper->SetScalarModeToUsePointData();
    mapper->SetInterpolationModeToCubic();
    mapper->SetCropping(false);
    mapper->SetBlendModeToComposite();
    mapper->SetMaxMemoryFraction(1);
    mapper->SetInputData(volume);
    mapper->Update();

    auto color = s_highlighter.color(m_colorEngine->color(segmentation), item->isSelected());

    auto colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorFunction->AllowDuplicateScalarsOff();
    colorFunction->AddHSVPoint(SEG_BG_VALUE, 0,0,0);
    colorFunction->AddHSVPoint(SEG_VOXEL_VALUE, color.hsvHueF(), color.hsvSaturationF(), color.valueF());
    colorFunction->SetAlpha(0);
    colorFunction->Modified();

    auto piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
    piecewise->AddPoint(SEG_BG_VALUE, 0.0);
    piecewise->AddPoint(SEG_VOXEL_VALUE, 1.0);
    piecewise->ClampingOff();
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
void SegmentationVolumetricGPUPipeline::updateColors(ActorList                 &actors,
                                                     ConstViewItemAdapterPtr    item,
                                                     const RepresentationState &state)
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

    auto actor = dynamic_cast<vtkVolume *>(actors.first().Get());

    auto property = dynamic_cast<vtkVolumeProperty *>(actor->GetProperty());

    auto transferFunction = property->GetRGBTransferFunction();

    transferFunction->AddHSVPoint(SEG_VOXEL_VALUE, color.hsvHueF(), color.hsvSaturationF(), color.valueF());

    transferFunction->Modified();
    property->Modified();
    actor->Update();
  }
}

//----------------------------------------------------------------------------
bool SegmentationVolumetricGPUPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
  return true;
}
