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

template<ESPINA::Plane T>
ESPINA::TransparencySelectionHighlighter ESPINA::SegmentationSlicePipeline<T>::s_highlighter;

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::SegmentationSlicePipeline<T>::SegmentationSlicePipeline()
: RepresentationPipeline("SegmentationSliceRepresentation")
{
}

#include <QDebug>
//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::RepresentationState SegmentationSlicePipeline<T>::representationState(const ViewItemAdapter *item,
                                                                           const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(SegmentationPipeline::Settings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
ESPINA::RepresentationPipeline::ActorList SegmentationSlicePipeline<T>::createActors(const ViewItemAdapter     *item,
                                                                                     const RepresentationState &state)
{
  auto segmentation = segmentationPtr(item);
  auto planeIndex = normalCoordinateIndex(s_plane);

  ActorList actors;
//   std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  Nm sliceNumber = crosshairPosition(s_plane, state);
//   qDebug() << "Slice:" << sliceNumber << "Createing Actors";
  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
//     qDebug() << "Slice:" << sliceNumber << "\t is visible";
    auto volume = volumetricData(segmentation->output());
      Bounds sliceBounds = volume->bounds();

      Nm reslicePoint = crosshairPosition(s_plane, state);

      if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint <= sliceBounds[2*planeIndex+1])
      {
//         qDebug() << "Slice:" << sliceNumber << "\t inside bounds";
        sliceBounds.setLowerInclusion(true);
        sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
        sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

        auto slice = vtkImage(volume, sliceBounds);

        auto color = segmentationColor(state);
        color.setRgb(255,0,0);
        auto mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
        mapToColors->SetInputData(slice);
        mapToColors->SetLookupTable(s_highlighter.lut(color));
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
//         qDebug() << "Pos: " << pos[planeIndex];
        pos[planeIndex] += segmentationDepth(state);
        actor->SetPosition(pos);

        actors << actor;
      }
  }

  return actors;
}

//----------------------------------------------------------------------------
template<ESPINA::Plane T>
bool ESPINA::SegmentationSlicePipeline<T>::pick(ViewItemAdapter *item, const NmVector3 &point) const
{
  bool result = false;

  auto output = item->output();
  if (hasVolumetricData(output) && contains(output->bounds(), point))
  {
    auto volume = volumetricData(output);
    auto voxel  = volume->itkImage(Bounds(point));

    result = *static_cast<unsigned char *>(voxel->GetBufferPointer()) == SEG_VOXEL_VALUE;
  }

  return result;
}
