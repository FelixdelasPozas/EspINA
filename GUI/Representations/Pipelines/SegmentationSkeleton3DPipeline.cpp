/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

IntensitySelectionHighlighter SegmentationSkeleton3DPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationSkeleton3DPipeline::SegmentationSkeleton3DPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationSkeleton3D"}
, m_colorEngine         {colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationSkeleton3DPipeline::representationState(ConstViewItemAdapterPtr    item,
                                                                        const RepresentationState &settings)
{
  RepresentationState state;

  auto segmentation = segmentationPtr(item);

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSkeleton3DPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                               const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);

  ActorList actors;

  if (isVisible(state) && hasSkeletonData(segmentation->output()))
  {
    auto data = readLockSkeleton(segmentation->output());

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(data->skeleton());
    mapper->Update();

    auto color = m_colorEngine->color(segmentation);
    double rgba[4];
    s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    auto width = item->isSelected() ? 4 : 2;

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    actor->GetProperty()->SetOpacity(opacity(state) * color.alphaF());
    actor->GetProperty()->SetLineWidth(width);
    actor->Modified();

    actors << actor;
  }

  return actors;
}

//----------------------------------------------------------------------------
bool SegmentationSkeleton3DPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // TODO
  return false;
}
