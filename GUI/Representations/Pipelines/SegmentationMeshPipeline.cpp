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
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Representations/Pipelines/SegmentationMeshPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkProperty.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

IntensitySelectionHighlighter SegmentationMeshPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationMeshPipeline::SegmentationMeshPipeline(ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationMesh"}
, m_colorEngine         {colorEngine}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationMeshPipeline::representationState(ConstViewItemAdapterPtr  item,
                                                                  const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationMeshPipeline::createActors(ConstViewItemAdapterPtr  item,
                                                                         const RepresentationState &state)
{
  ActorList actors;

  auto segmentation = segmentationPtr(item);

  if(isVisible(state) && hasMeshData(segmentation->output()))
  {
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->ReleaseDataFlagOn();
    mapper->ImmediateModeRenderingOff();
    mapper->ScalarVisibilityOff();
    mapper->SetInputData(readLockMesh(segmentation->output())->mesh());
    mapper->Update();

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetSpecular(0.2);
    actor->GetProperty()->SetOpacity(1);
    actor->Modified();

    actors << actor;
  }

  return actors;
}

//----------------------------------------------------------------------------
void SegmentationMeshPipeline::updateColors(ActorList& actors,
                                            ConstViewItemAdapterPtr    item,
                                            const RepresentationState& state)
{
  if (actors.size() == 1)
  {
    auto segmentation = segmentationPtr(item);

    auto color = s_highlighter.color(m_colorEngine->color(segmentation), item->isSelected());

    auto actor = dynamic_cast<vtkActor *>(actors.first().Get());

    actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
  }
}

//----------------------------------------------------------------------------
bool SegmentationMeshPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // relies on an actor being picked in the View3D and the updater selecting the correct ViewItem.
  return true;
}
