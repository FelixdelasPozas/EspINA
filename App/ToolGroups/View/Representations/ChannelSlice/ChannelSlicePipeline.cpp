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
, m_channelSettings(dynamic_cast<ChannelAdapterPtr>(item))
, m_planeIndex(normalCoordinateIndex(s_plane))
{
  initPipeline();
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ChannelSlicePipeline<T>::pick(const NmVector3 &point, vtkProp *actor)
{
  return false;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ChannelSlicePipeline<T>::update()
{
  m_actors.clear();

  if (!m_state.getValue<bool>(VISIBLE)) return;

  auto channel = m_channelSettings.channel();

  if (!hasVolumetricData(channel->output())) return;

  Nm reslicePoint = crosshairPosition(s_plane);

  auto data = volumetricData(channel->output());

  vtkSmartPointer<vtkImageData> slice;

  bool dataChanged = data->lastModified() != m_state.getValue<TimeStamp>(TIME_STAMP);
  bool crosshairPositionChanged = isCrosshairPositionModified(s_plane);
  if (crosshairPositionChanged || dataChanged)
  {
    Bounds imageBounds = data->bounds();

    if (reslicePoint < imageBounds[2*m_planeIndex]
     || reslicePoint > imageBounds[2*m_planeIndex+1]) return;

    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);
    imageBounds[2*m_planeIndex] = imageBounds[2*m_planeIndex+1] = reslicePoint;

    slice = vtkImage(data, imageBounds);

    m_shiftScaleFilter->SetInputData(slice);

    if (dataChanged)
    {
      m_state.setValue<TimeStamp>(TIME_STAMP, data->lastModified());
    }
  }

  bool brightnessChanged = m_state.isModified(BRIGHTNESS);
  if (brightnessChanged)
  {
    updateBrightness();
  }

  bool contrastChanged = m_state.isModified(CONTRAST);
  if (contrastChanged)
  {
    updateContrast();
  }

  bool stainChanged = m_state.isModified(STAIN);
  if (stainChanged)
  {
    updateStain();
  }

  if (crosshairPositionChanged || brightnessChanged || contrastChanged)
  {
    m_shiftScaleFilter->Update();
  }

  if (stainChanged)
  {
    m_lut->Build();
    m_mapToColors->Update();
  }

  //m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  if (crosshairPositionChanged)
  {
    m_actor->SetDisplayExtent(slice->GetExtent());
  }

  if (crosshairPositionChanged || brightnessChanged || contrastChanged || stainChanged)
  {
    m_actor->Update();
    m_state.commit();
  }
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
QList<ESPINA::RepresentationPipeline::Actor> ESPINA::ChannelSlicePipeline<T>::getActors()
{
  return m_actors;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::applySettings(const Settings &settings)
{
  m_state.apply(m_channelSettings.settings());
  m_state.apply(settings);
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::initPipeline()
{
  auto channel = m_channelSettings.channel();

  if (!hasVolumetricData(channel->output())) return;

  int idx = normalCoordinateIndex(s_plane);

  Nm reslicePoint = crosshairPosition(s_plane);

  auto m_data = volumetricData(channel->output());

  Bounds imageBounds = m_data->bounds();

  bool valid = imageBounds[2*idx] <= reslicePoint && reslicePoint <= imageBounds[2*idx+1];

  vtkSmartPointer<vtkImageData> slice;

  if (valid)
  {
    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(idx), true);
    imageBounds[2*idx] = imageBounds[(2*idx)+1] = reslicePoint;

    slice = vtkImage(m_data, imageBounds);
  }
  else
  {
    int extent[6] = { 0,1,0,1,0,1 };
    extent[2*idx + 1] = extent[2*idx];

    slice = vtkSmartPointer<vtkImageData>::New();
    slice->SetExtent(extent);

    auto info = slice->GetInformation();
    vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
    vtkImageData::SetNumberOfScalarComponents(1, info);
    slice->SetInformation(info);
    slice->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    slice->Modified();

    auto imagePointer = reinterpret_cast<unsigned char*>(slice->GetScalarPointer());
    memset(imagePointer, SEG_BG_VALUE, slice->GetNumberOfPoints());
  }

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
void ESPINA::ChannelSlicePipeline<T>::updateBrightness()
{
  m_shiftScaleFilter->SetShift(static_cast<int>(m_state.getValue<double>(BRIGHTNESS)*255));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateContrast()
{
  m_shiftScaleFilter->SetScale(m_state.getValue<double>(CONTRAST));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateStain()
{
  auto stain = m_state.getValue<QColor>(STAIN);

  m_lut->SetHueRange(stain.hueF(), stain.hueF());
  m_lut->SetSaturationRange(0.0, stain.saturationF());
}
