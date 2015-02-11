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
ESPINA::SegmentationSlicePipeline<T>::SegmentationSlicePipeline()
: RepresentationPipeline("SegmentationSliceRepresentation")
{
}

#include <QDebug>
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::SegmentationSlicePipeline<T>::applySettingsImplementation(const Settings &settings)
{
  auto segmentation = segmentationPtr(item);

  state.apply(SegmentationPipeline::Settings(segmentation));
  state.apply(settings);

  return state.hasPendingChanges();
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::RepresentationPipeline::ActorList ESPINA::SegmentationSlicePipeline::createActors(ESPINA::ViewItemAdapter *item, const ESPINA::RepresentationState &state)
{
  auto segmentation = segmentationPtr(item);
  auto planeIndex = normalCoordinateIndex(s_plane);

  ActorList actors;
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  Nm sliceNumber = crosshairPosition(s_plane, state);
  qDebug() << "Slice:" << sliceNumber << "Createing Actors";
  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    qDebug() << "Slice:" << sliceNumber << "\t is visible";
    auto volume = volumetricData(segmentation->output());
      Bounds sliceBounds = volume->bounds();

      Nm reslicePoint = crosshairPosition(s_plane, state);

      if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint <= sliceBounds[2*planeIndex+1])
      {
        qDebug() << "Slice:" << sliceNumber << "\t inside bounds";
        sliceBounds.setLowerInclusion(true);
        sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
        sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

        auto slice = vtkImage(volume, sliceBounds);

        auto color = segmentationColor(state);
        auto mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
        mapToColors->SetInputData(slice);
        mapToColors->SetLookupTable(s_highlighter->lut(color));
        mapToColors->SetNumberOfThreads(1);
        mapToColors->Update();

        auto actor = vtkSmartPointer<vtkImageActor>::New();
        actor->SetInterpolate(false);
        actor->GetMapper()->BorderOn();
        actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
        actor->SetDisplayExtent(slice->GetExtent());
        actor->Update();

        // need to reposition the actor so it will always be over the channels actors'
        double pos[3];
        actor->GetPosition(pos);
        qDebug() << "Pos: " << pos[planeIndex];
        pos[planeIndex] += segmentationDepth(state);
        actor->SetPosition(pos);

        actors << actor;
      }
  }

  return actors;
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

  double pos[3];
  m_actor->GetPosition(pos);
  std::cout << "Pos: " << pos[m_planeIndex] << std::endl;
//   pos[m_planeIndex] += m_view->segmentationDepth();
//   m_actor->SetPosition(pos);
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
void ESPINA::SegmentationSlicePipeline<T>::updateColor()
{
  auto color = state<QColor>(COLOR);

//   m_lut->SetHueRange(stain.hueF(), stain.hueF());
//   m_lut->SetSaturationRange(0.0, stain.saturationF());
}
