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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/BlockTimer.hxx>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Representations/Pipelines/ChannelSlicePipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkAlgorithmOutput.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
ChannelSlicePipeline::ChannelSlicePipeline(const Plane plane)
: RepresentationPipeline("ChannelSliceRepresentation")
, m_plane{plane}
{
}

//----------------------------------------------------------------------------
RepresentationState ChannelSlicePipeline::representationState(ConstViewItemAdapterPtr   item,
                                                              const RepresentationState &settings)
{
  RepresentationState state;

  auto channel = channelPtr(item);

  state.apply(channelPipelineSettings(channel));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList ChannelSlicePipeline::createActors(ConstViewItemAdapterPtr   item,
                                                                     const RepresentationState &state)
{
  auto channel    = channelPtr(item);
  auto planeIndex = normalCoordinateIndex(m_plane);

  ActorList actors;

  // BlockTimer<> timer("Channel representation pipeline");

  if (isVisible(state) && hasVolumetricData(channel->output()))
  {
    auto reslicePoint  = crosshairPosition(m_plane, state);
    Bounds sliceBounds = item->bounds();

    if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
    {
      sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
      sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

      // solid slice
      auto slice = vtkImage(readLockVolume(channel->output(), DataUpdatePolicy::Ignore), sliceBounds);
      int extent[6];
      slice->GetExtent(extent);

      auto shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
      shiftScaleFilter->SetInputData(slice);
      shiftScaleFilter->SetNumberOfThreads(1);
      shiftScaleFilter->SetShift(static_cast<int>(brightness(state)*255));
      shiftScaleFilter->SetScale(contrast(state));
      shiftScaleFilter->SetClampOverflow(true);
      shiftScaleFilter->SetOutputScalarType(slice->GetScalarType());
      shiftScaleFilter->UpdateWholeExtent();

      auto color = stain(state);
      auto lut = vtkSmartPointer<vtkLookupTable>::New();
      lut->Allocate();
      lut->SetTableRange(0,255);
      lut->SetValueRange(0.0, 1.0);
      lut->SetAlphaRange(1.0,1.0);
      lut->SetNumberOfColors(256);
      lut->SetRampToLinear();
      lut->SetHueRange(color.hueF(), color.hueF());
      lut->SetSaturationRange(0.0, color.saturationF());
      lut->Build();

      auto mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
      mapToColors->SetInputConnection(shiftScaleFilter->GetOutputPort());
      mapToColors->SetLookupTable(lut);
      mapToColors->SetNumberOfThreads(1);
      mapToColors->UpdateWholeExtent();

      auto actor = vtkSmartPointer<vtkImageActor>::New();
      actor->SetInterpolate(false);
      actor->GetMapper()->BorderOn();
      actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
      actor->GetMapper()->SetNumberOfThreads(1);
      actor->GetMapper()->UpdateWholeExtent();
      actor->SetDisplayExtent(extent);
      actor->SetOpacity(opacity(state));
      actor->Update();

      actors << actor;
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
void ChannelSlicePipeline::updateColors(ActorList& actors,
                                        ConstViewItemAdapterPtr   item,
                                        const RepresentationState &state)
{
  if (actors.size() == 1)
  {
    auto actor       = vtkImageActor::SafeDownCast(actors.first().Get());
    auto mapToColors = vtkImageMapToColors::SafeDownCast(actor->GetMapper()->GetInputConnection(0,0)->GetProducer());

    auto color = stain(state);
    auto lut   = vtkLookupTable::SafeDownCast(mapToColors->GetLookupTable());

    lut->SetHueRange(color.hueF(), color.hueF());
    lut->SetSaturationRange(0.0, color.saturationF());
  }
}

//----------------------------------------------------------------------------
bool ChannelSlicePipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  return contains(item->output()->bounds(), point);
}

//----------------------------------------------------------------------------
void ChannelSlicePipeline::setPlane(const Plane plane)
{
  m_plane = plane;
}
