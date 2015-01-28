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

#include "SegmentationSlicePipeline.h"

template<ESPINA::Plane T>
ESPINA::Plane ESPINA::SegmentationSlicePipeline<T>::s_plane = T;

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::SegmentationSlicePipeline<T>::SegmentationSlicePipeline(ViewItemAdapterPtr item)
: RepresentationPipeline("SegmentationSliceRepresentation")
, m_segmentation(dynamic_cast<SegmentationAdapterPtr>(item))
, m_planeIndex(normalCoordinateIndex(s_plane))
{
  initPipeline();
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool SegmentationSlicePipeline<T>::pick(const NmVector3 &point, vtkProp *actor)
{
  return false;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void SegmentationSlicePipeline<T>::update()
{
  m_actors.clear();

  if (!m_state.getValue<bool>(VISIBLE)) return;

  if (!hasVolumetricData(m_segmentation->output())) return;

  Nm reslicePoint = crosshairPosition(s_plane);

  auto data = volumetricData(m_segmentation->output());

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

    m_mapToColors->SetInputData(slice);

    if (dataChanged)
    {
      m_state.setValue<TimeStamp>(TIME_STAMP, data->lastModified());
    }
  }

  bool colorChanged = m_state.isModified(COLOR);
  if (colorChanged)
  {
    updateColor();
  }

  // TODO: Simplify if conditions if no more changes are done
  if (crosshairPositionChanged || colorChanged)
  {
    m_mapToColors->Update();
  }

  if (crosshairPositionChanged)
  {
    m_actor->SetDisplayExtent(slice->GetExtent());
  }

  if (crosshairPositionChanged || colorChanged)
  {
    m_actor->Update();
    m_state.commit();
  }

  m_actors << m_actor;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
QList<ESPINA::RepresentationPipeline::Actor> ESPINA::SegmentationSlicePipeline<T>::getActors()
{
  return m_actors;
}

#include <QDebug>
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::SegmentationSlicePipeline<T>::applySettings(const Settings &settings)
{
  m_state.apply(SegmentationPipeline::Settings(m_segmentation));
  m_state.apply(settings);
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::SegmentationSlicePipeline<T>::initPipeline()
{
  if (!hasVolumetricData(m_segmentation->output())) return;

  int idx = normalCoordinateIndex(s_plane);

  Nm reslicePoint = crosshairPosition(s_plane);

  auto m_data = volumetricData(m_segmentation->output());

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

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputData(slice);
  updateColor();
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->SetDisplayExtent(slice->GetExtent());
  m_actor->Update();
  // need to reposition the actor so it will always be over the channels actors'

  // TODO
//   double pos[3];
//   m_actor->GetPosition(pos);
//   pos[m_planeIndex] += view->segmentationDepth();
//   m_actor->SetPosition(pos);
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::SegmentationSlicePipeline<T>::updateColor()
{
  auto color = m_state.getValue<QColor>(COLOR);

//   m_lut->SetHueRange(stain.hueF(), stain.hueF());
//   m_lut->SetSaturationRange(0.0, stain.saturationF());
}
