/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/Utils/RepresentationUtils.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

// VTK
#include <vtkActor.h>
#include <vtkProperty.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::RepresentationUtils;

IntensitySelectionHighlighter SegmentationSkeletonPipelineBase::s_highlighter;

//--------------------------------------------------------------------
SegmentationSkeletonPipelineBase::SegmentationSkeletonPipelineBase(const QString &id, ColorEngineSPtr colorEngine)
: RepresentationPipeline{id}
, m_colorEngine         {colorEngine}
{
}

//--------------------------------------------------------------------
void SegmentationSkeletonPipelineBase::updateColors(RepresentationPipeline::ActorList& actors,
                                                    ConstViewItemAdapterPtr            item,
                                                    const RepresentationState&         state)
{
  auto segmentation = segmentationPtr(item);
  if(!segmentation) return;

  for(auto actor: actors)
  {
    auto actorVTK = vtkActor::SafeDownCast(actor.Get());

    if(actorVTK)
    {
      auto color = m_colorEngine->color(segmentation);
      double rgba[4];
      s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

      actorVTK->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
      actorVTK->GetProperty()->SetOpacity(opacity(state) * rgba[3]);

      auto width = item->isSelected() ? 2 : 1;
      width *= SegmentationSkeletonPoolSettings::getWidth(state);

      actorVTK->GetProperty()->SetLineWidth(width);
      actorVTK->Modified();
    }
  }
}

//--------------------------------------------------------------------
bool SegmentationSkeletonPipelineBase::pick(ConstViewItemAdapterPtr item, const NmVector3& point) const
{
  // TODO
  return false;
}

//--------------------------------------------------------------------
RepresentationState SegmentationSkeletonPipelineBase::representationState(ConstViewItemAdapterPtr    item,
                                                                          const RepresentationState &settings)
{
  RepresentationState state;

  auto segmentation = segmentationPtr(item);

  if(segmentation)
  {
    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);
  }

  return state;
}
