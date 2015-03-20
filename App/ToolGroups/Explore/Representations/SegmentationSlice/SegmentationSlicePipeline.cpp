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
#include "Support/Representations/RepresentationUtils.h"

#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::RepresentationUtils;

TransparencySelectionHighlighter SegmentationSlicePipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationSlicePipeline::SegmentationSlicePipeline(const Plane plane,
                                                     ColorEngineSPtr colorEngine)
: RepresentationPipeline("SegmentationSliceRepresentation")
, m_plane{plane}
, m_colorEngine{colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationSlicePipeline::representationState(const ViewItemAdapter *item,
                                                                   const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(SegmentationPipeline::Settings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSlicePipeline::createActors(const ViewItemAdapter     *item,
                                                                          const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);
  auto planeIndex   = normalCoordinateIndex(m_plane);

  ActorList actors;
//   std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    auto volume = volumetricData(segmentation->output());
    Bounds sliceBounds = volume->bounds();

    Nm reslicePoint = crosshairPosition(m_plane, state);

    if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
    {
      sliceBounds.setLowerInclusion(true);
      sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
      sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

      auto slice = vtkImage(volume, sliceBounds);

      auto color       = m_colorEngine->color(segmentation);
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
      pos[planeIndex] += segmentationDepth(state);
      actor->SetPosition(pos);

      actors << actor;
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
bool SegmentationSlicePipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
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

//----------------------------------------------------------------------------
void SegmentationSlicePipeline::setPlane(const Plane plane)
{
  m_plane = plane;
}
