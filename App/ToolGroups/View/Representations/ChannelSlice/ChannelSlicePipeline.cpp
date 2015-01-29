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

template<ESPINA::Plane T>
ESPINA::Plane ESPINA::ChannelSlicePipeline<T>::s_plane = T;

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::ChannelSlicePipeline<T>::ChannelSlicePipeline(ViewItemAdapterPtr item)
: RepresentationPipeline("ChannelSliceRepresentation")
, m_channel(dynamic_cast<ChannelAdapterPtr>(item))
, m_planeIndex(normalCoordinateIndex(s_plane))
{
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ChannelSlicePipeline<T>::pick(const NmVector3 &point, vtkProp *actor)
{
  return false;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
QList<ESPINA::RepresentationPipeline::Actor> ESPINA::ChannelSlicePipeline<T>::getActors()
{
  return m_actors;
}

#include <QDebug>
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::applySettingsImplementation(const Settings &settings)
{
  updateState(ChannelPipeline::Settings(m_channel));
  updateState(settings);
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ChannelSlicePipeline<T>::updateImplementation()
{
  m_actors.clear();

  if (!state<bool>(VISIBLE)) return false;

  if (!hasVolumetricData(m_channel->output())) return false;

  Nm reslicePoint = crosshairPosition(s_plane);

  auto volume = volumetricData(m_channel->output());

  vtkSmartPointer<vtkImageData> slice;

  bool dataChanged = volume->lastModified() != state<TimeStamp>(TIME_STAMP);
  bool crosshairPositionChanged = isCrosshairPositionModified(s_plane);
  if (crosshairPositionChanged || dataChanged)
  {
    Bounds sliceBounds = volume->bounds();

    if (dataChanged)
    {
      setState<TimeStamp>(TIME_STAMP, volume->lastModified());
    }

    if (reslicePoint < sliceBounds[2*m_planeIndex]
     || reslicePoint > sliceBounds[2*m_planeIndex+1]) return true; // Not visible

    sliceBounds.setLowerInclusion(true);
    sliceBounds.setUpperInclusion(toAxis(m_planeIndex), true);
    sliceBounds[2*m_planeIndex] = sliceBounds[2*m_planeIndex+1] = reslicePoint;

    createPipeline(volume, sliceBounds);
  }
  else
  {
    updatePipeline();
  }

  m_actors << m_actor;

  return true;
}
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::createPipeline(DefaultVolumetricDataSPtr volume,
                                                     const Bounds             &sliceBounds)
{
  auto slice = vtkImage(volume, sliceBounds);

  m_shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  m_shiftScaleFilter->SetInputData(slice);
  updateBrightness();
  updateContrast();
  m_shiftScaleFilter->SetClampOverflow(true);
  m_shiftScaleFilter->SetOutputScalarType(slice->GetScalarType());
  m_shiftScaleFilter->Update();

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetTableRange(0,255);
  m_lut->SetValueRange(0.0, 1.0);
  m_lut->SetAlphaRange(1.0,1.0);
  m_lut->SetNumberOfColors(256);
  m_lut->SetRampToLinear();
  updateStain();
  m_lut->Build();

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputConnection(m_shiftScaleFilter->GetOutputPort());
  m_mapToColors->SetLookupTable(m_lut);
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->SetDisplayExtent(slice->GetExtent());
  m_actor->Update();
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updatePipeline()
{
  bool brightnessChanged = isModified(BRIGHTNESS);
  if (brightnessChanged)
  {
    updateBrightness();
  }

  bool contrastChanged = isModified(CONTRAST);
  if (contrastChanged)
  {
    updateContrast();
  }

  bool stainChanged = isModified(STAIN);
  if (stainChanged)
  {
    updateStain();
  }

  if (brightnessChanged || contrastChanged)
  {
    m_shiftScaleFilter->Update();
  }

  if (stainChanged)
  {
    m_lut->Build();
    m_mapToColors->Update();
  }

  if (brightnessChanged || contrastChanged || stainChanged)
  {
    m_actor->Update();
  }
}
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateBrightness()
{
  m_shiftScaleFilter->SetShift(static_cast<int>(state<double>(BRIGHTNESS)*255));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateContrast()
{
  m_shiftScaleFilter->SetScale(state<double>(CONTRAST));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateStain()
{
  auto stain = state<QColor>(STAIN);

  m_lut->SetHueRange(stain.hueF(), stain.hueF());
  m_lut->SetSaturationRange(0.0, stain.saturationF());
}
