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
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;


//----------------------------------------------------------------------------
SegmentationSkeleton3DPipeline::SegmentationSkeleton3DPipeline(ColorEngineSPtr colorEngine)
: SegmentationSkeletonPipelineBase{"SegmentationSkeleton3D", colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSkeleton3DPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                               const RepresentationState &state)
{
  auto segmentation = segmentationPtr(item);

  ActorList actors;

  if (segmentation && isVisible(state) && hasSkeletonData(segmentation->output()))
  {
    auto data = readLockSkeleton(segmentation->output());

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(data->skeleton());
    mapper->Update();

    auto color = m_colorEngine->color(segmentation);
    double rgba[4];
    SegmentationSkeletonPipelineBase::s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    auto width = item->isSelected() ? 2 : 1;
    width *= SegmentationSkeletonPoolSettings::getWidth(state);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    actor->GetProperty()->SetOpacity(opacity(state) * color.alphaF());
    actor->GetProperty()->SetLineWidth(width);
    actor->Modified();

    actors << actor;

    if(SegmentationSkeletonPoolSettings::getShowAnnotations(state))
    {
      // TODO annotations actor.
    }
  }

  return actors;
}
