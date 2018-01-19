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
#include <GUI/Representations/Pipelines/SegmentationSlicePipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/View/Utils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Utils/RepresentationUtils.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkAlgorithmOutput.h>

using namespace ESPINA;
using namespace ESPINA::GUI::RepresentationUtils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::View::Utils;

IntensitySelectionHighlighter SegmentationSlicePipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationSlicePipeline::SegmentationSlicePipeline(const Plane plane, ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationSliceRepresentation"}
, m_plane               {plane}
, m_colorEngine         {colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationSlicePipeline::representationState(const ViewItemAdapter     *item,
                                                                   const RepresentationState &settings)
{
  RepresentationState state;

  if(!item) return state;

  auto segmentation = segmentationPtr(item);

  if(!segmentation) return state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSlicePipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                          const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);
  auto planeIndex   = normalCoordinateIndex(m_plane);

  ActorList actors;
//   std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    Bounds sliceBounds = readLockVolume(segmentation->output())->bounds();

    Nm reslicePoint = crosshairPosition(m_plane, state);

    if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
    {
      sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
      sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

      auto slice = vtkImage(readLockVolume(segmentation->output()), sliceBounds);

      addPadding(slice, planeIndex);

      int extent[6];
      slice->GetExtent(extent);

      auto color       = m_colorEngine->color(segmentation);
      auto mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
      mapToColors->SetInputData(slice);
      mapToColors->SetLookupTable(s_highlighter.lut(color, item->isSelected()));
      mapToColors->SetUpdateExtent(extent);
      mapToColors->SetNumberOfThreads(1);
      mapToColors->UpdateInformation();
      mapToColors->UpdateWholeExtent();

      auto actor = vtkSmartPointer<vtkImageActor>::New();
      actor->GetMapper()->BorderOn();
      actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
      actor->GetMapper()->SetUpdateExtent(extent);
      actor->GetMapper()->SetNumberOfThreads(1);
      actor->GetMapper()->UpdateInformation();
      actor->GetMapper()->UpdateWholeExtent();
      actor->SetOpacity(opacity(state) * color.alphaF());
      actor->SetPickable(false);
      actor->SetInterpolate(false);
      actor->SetDisplayExtent(extent);
      actor->Update();

      // need to reposition the actor so it will always be over the channels actors'
      repositionActor(actor, segmentationDepth(state), planeIndex);

      actors << actor;
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
void SegmentationSlicePipeline::updateColors(ActorList                 &actors,
                                             const ViewItemAdapter     *item,
                                             const RepresentationState &state)
{
  if (actors.size() == 1)
  {
    auto segmentation = segmentationPtr(item);

    auto actor = vtkImageActor::SafeDownCast(actors.first().Get());
    auto color = m_colorEngine->color(segmentation);

    actor->SetOpacity(opacity(state) * color.alphaF());

    auto mapToColors = vtkImageMapToColors::SafeDownCast(actor->GetMapper()->GetInputConnection(0,0)->GetProducer());
    mapToColors->SetLookupTable(s_highlighter.lut(color, item->isSelected()));
  }
}

//----------------------------------------------------------------------------
bool SegmentationSlicePipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  if(hasVolumetricData(item->output()))
  {
    auto volume = readLockVolume(item->output());
    return isSegmentationVoxel(volume, point);
  }

  return false;
}

//----------------------------------------------------------------------------
void SegmentationSlicePipeline::setPlane(const Plane plane)
{
  m_plane = plane;
}
