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

#include "ChannelSlicePipeline.h"

#include <QDebug>

template<ESPINA::Plane T>
ESPINA::Plane ESPINA::ChannelSlicePipeline<T>::s_plane = T;

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::ChannelSlicePipeline<T>::ChannelSlicePipeline()
: RepresentationPipeline("ChannelSliceRepresentation")
{
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ESPINA::ChannelSlicePipeline<T>::applySettings(ViewItemAdapter           *item,
                                                 const RepresentationState &settings,
                                                 RepresentationState       &state)
{
  auto channel = channelPtr(item);

  state.apply(ChannelPipeline::Settings(channel));
  state.apply(settings);

  return state.hasPendingChanges();
}


//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::RepresentationPipeline::ActorList ChannelSlicePipeline<T>::createActors(ViewItemAdapter           *item,
                                                                                const RepresentationState &state)
{
  auto channel    = channelPtr(item);
  auto planeIndex = normalCoordinateIndex(s_plane);

  ActorList actors;
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  Nm sliceNumber = crosshairPosition(s_plane, state);
  qDebug() << "Slice:" << sliceNumber << "Createing Actors";
  if (isVisible(state) && hasVolumetricData(channel->output()))
  {
    qDebug() << "Slice:" << sliceNumber << "\t is visible";
    auto volume = volumetricData(channel->output());
    Bounds sliceBounds = volume->bounds();

    Nm reslicePoint = crosshairPosition(s_plane, state);

    if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint <= sliceBounds[2*planeIndex+1])
    {
      qDebug() << "Slice:" << sliceNumber << "\t inside bounds";
      sliceBounds.setLowerInclusion(true);
      sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
      sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

      //    createPipeline(volume, sliceBounds, settings);
      auto slice = vtkImage(volume, sliceBounds);

      auto shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
      shiftScaleFilter->SetInputData(slice);
      shiftScaleFilter->SetShift(static_cast<int>(brightness(state)*255));
      shiftScaleFilter->SetScale(contrast(state));
      shiftScaleFilter->SetClampOverflow(true);
      shiftScaleFilter->SetOutputScalarType(slice->GetScalarType());
      shiftScaleFilter->Update();

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
      mapToColors->Update();

      auto actor = vtkSmartPointer<vtkImageActor>::New();
      actor->SetInterpolate(false);
      actor->GetMapper()->BorderOn();
      actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
      actor->SetDisplayExtent(slice->GetExtent());
      actor->Update();

      actors << actor;
    }
  }
  qDebug() << "Slice:" << sliceNumber << "\t has" << actors.size() <<"actors";

  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

  if (s_plane == Plane::YZ) qDebug() <<  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  return actors;
 }

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ChannelSlicePipeline<T>::pick(const NmVector3 &point, vtkProp *actor)
{
  return false;
}