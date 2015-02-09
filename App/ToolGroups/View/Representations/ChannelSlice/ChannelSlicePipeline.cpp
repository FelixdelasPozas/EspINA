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
, m_timeStamp(VTK_UNSIGNED_LONG_LONG_MAX)
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
bool ChannelSlicePipeline<T>::updateImplementation(const Settings &settings)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  m_actors.clear();

  if (!settings.getValue<bool>(VISIBLE)) return false;

  if (!hasVolumetricData(m_channel->output())) return false;

  auto volume = volumetricData(m_channel->output());

  bool dataChanged = volume->lastModified() != m_timeStamp;
  bool crosshairPositionChanged = isCrosshairPositionModified(s_plane, settings);
  if (crosshairPositionChanged || dataChanged)
  {
    Bounds sliceBounds = volume->bounds();

    if (dataChanged)
    {
      m_timeStamp = volume->lastModified();
    }

    Nm reslicePoint = crosshairPosition(s_plane, settings);

    if (reslicePoint < sliceBounds[2*m_planeIndex]
     || reslicePoint > sliceBounds[2*m_planeIndex+1]) return true; // Not visible

//     qDebug() << "Slice" <<  reslicePoint << "running";
    sliceBounds.setLowerInclusion(true);
    sliceBounds.setUpperInclusion(toAxis(m_planeIndex), true);
    sliceBounds[2*m_planeIndex] = sliceBounds[2*m_planeIndex+1] = reslicePoint;

    createPipeline(volume, sliceBounds, settings);

//     qDebug() << "Slice" <<  reslicePoint << "finished";
  }
  else
  {
    updatePipeline(settings);
  }

  m_actors << m_actor;

  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

  if (s_plane == Plane::YZ) qDebug() <<  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  return true;
}
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::createPipeline(DefaultVolumetricDataSPtr volume,
                                                     const Bounds             &sliceBounds,
                                                     const Settings           &settings)
{
  auto slice = vtkImage(volume, sliceBounds);

  m_shiftScaleFilter = vtkSmartPointer<vtkImageShiftScale>::New();
  m_shiftScaleFilter->SetInputData(slice);
  updateBrightness(settings);
  updateContrast(settings);
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
  updateStain(settings);
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
void ESPINA::ChannelSlicePipeline<T>::updatePipeline(const Settings &settings)
{
  bool brightnessChanged = settings.isModified(BRIGHTNESS);
  if (brightnessChanged)
  {
    updateBrightness(settings);
  }

  bool contrastChanged = settings.isModified(CONTRAST);
  if (contrastChanged)
  {
    updateContrast(settings);
  }

  bool stainChanged = settings.isModified(STAIN);
  if (stainChanged)
  {
    updateStain(settings);
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
void ESPINA::ChannelSlicePipeline<T>::updateBrightness(const Settings &settings)
{
  m_shiftScaleFilter->SetShift(static_cast<int>(settings.getValue<double>(BRIGHTNESS)*255));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateContrast(const Settings &settings)
{
  m_shiftScaleFilter->SetScale(settings.getValue<double>(CONTRAST));
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::ChannelSlicePipeline<T>::updateStain(const Settings &settings)
{
  auto stain = settings.getValue<QColor>(STAIN);

  m_lut->SetHueRange(stain.hueF(), stain.hueF());
  m_lut->SetSaturationRange(0.0, stain.saturationF());
}
