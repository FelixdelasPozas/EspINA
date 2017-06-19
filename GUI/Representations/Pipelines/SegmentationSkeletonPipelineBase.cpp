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
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/Utils/RepresentationUtils.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

// VTK
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkActor2D.h>

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

      auto width = item->isSelected() ? 2 : 1;
      width *= SegmentationSkeletonPoolSettings::getWidth(state);

      actorVTK->GetProperty()->SetLineWidth(width);
      actorVTK->Modified();
    }

    auto actor2D = vtkActor2D::SafeDownCast(actor.Get());

    if(actor2D)
    {
      actor2D->SetVisibility(item->isSelected());
    }
  }
}

//--------------------------------------------------------------------
bool SegmentationSkeletonPipelineBase::pick(ConstViewItemAdapterPtr item, const NmVector3& point) const
{
  if(hasSkeletonData(item->output()))
  {
    auto skeleton = readLockSkeleton(item->output())->skeleton();
    auto lines = skeleton->GetLines();
    double projection[3];
    auto points = skeleton->GetPoints();
    long long npts;
    long long *pts;

    // NOTE to self: vtkLine::DistanceToLine(p0, lineP0, lineP1, t, closest) is pure shit. Thank god for a little
    // knowledge of computational geometry.
    lines->InitTraversal();
    while(lines->GetNextCell(npts, pts))
    {
      for(int i = 0; i < npts-1; ++i)
      {
        double pos_i[3], pos_j[3];
        points->GetPoint(pts[i], pos_i);
        points->GetPoint(pts[i+1], pos_j);

        double v[3]{pos_j[0]-pos_i[0], pos_j[1]-pos_i[1], pos_j[2]-pos_i[2]};
        double w[3]{point[0]-pos_i[0], point[1]-pos_i[1], point[2]-pos_i[2]};

        double dotwv = 0;
        double dotvv = 0;
        for(auto ii: {0,1,2})
        {
          dotwv += w[ii]*v[ii];
          dotvv += v[ii]*v[ii];
        }

        double r = dotwv / dotvv;

        if(r <= 0)
        {
          std::memcpy(projection, pos_i, 3*sizeof(double));
        }
        else
        {
          if(r >= 1)
          {
            std::memcpy(projection, pos_j, 3*sizeof(double));
          }
          else
          {
            projection[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
            projection[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
            projection[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);
          }
        }

        double distance = std::pow(projection[0] - point[0], 2) + std::pow(projection[1] - point[1], 2) + std::pow(projection[2] - point[2], 2);

        if(distance < 10) return true;
      }
    }
  }

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
