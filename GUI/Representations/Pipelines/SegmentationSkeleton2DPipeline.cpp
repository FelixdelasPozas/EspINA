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
#include <GUI/Representations/Pipelines/SegmentationSkeleton2DPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <Support/Representations/RepresentationUtils.h>

// VTK
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

namespace ESPINA
{
  TransparencySelectionHighlighter SegmentationSkeleton2DPipeline::s_highlighter;

  //----------------------------------------------------------------------------
  SegmentationSkeleton2DPipeline::SegmentationSkeleton2DPipeline(Plane plane, ColorEngineSPtr colorEngine)
  : RepresentationPipeline{"SegmentationSkeleton2D"}
  , m_plane               {plane}
  , m_colorEngine         {colorEngine}
  {
  }
  
  //----------------------------------------------------------------------------
  RepresentationState SegmentationSkeleton2DPipeline::representationState(const ViewItemAdapter     *item,
                                                                          const RepresentationState &settings)
  {
    RepresentationState state;

    auto segmentation = segmentationPtr(item);

    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);
    
    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList SegmentationSkeleton2DPipeline::createActors(const ViewItemAdapter     *item,
                                                                                 const RepresentationState &state)
  {
    auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);
    auto planeIndex   = normalCoordinateIndex(m_plane);

    ActorList actors;

    if (isVisible(state) && hasSkeletonData(segmentation->output()))
    {
      auto data = skeletonData(segmentation->output());
      Bounds sliceBounds = data->bounds();

      Nm reslicePoint = crosshairPosition(m_plane, state);

      if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
      {
        auto newPoints = vtkSmartPointer<vtkPoints>::New();
        auto newLines = vtkSmartPointer<vtkCellArray>::New();

        auto skeleton = data->skeleton();
        auto planeSpacing = data->spacing()[planeIndex];

        QMap<vtkIdType, NmVector3> pointIds;
        QMap<vtkIdType, vtkIdType> newPointIds;
        auto points = skeleton->GetPoints();
        auto lines = skeleton->GetLines();
        double pointACoords[3]{0,0,0};
        double pointBCoords[3]{0,0,0};

        lines->InitTraversal();
        vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
        auto sliceDepth = reslicePoint + RepresentationUtils::segmentationDepth(state);
        while(lines->GetNextCell(idList))
        {
          if(idList->GetNumberOfIds() != 2)
          {
            continue;
          }

          vtkIdType pointAId = idList->GetId(0);
          vtkIdType pointBId = idList->GetId(1);
          points->GetPoint(pointAId, pointACoords);
          points->GetPoint(pointBId, pointBCoords);

          if((pointACoords[planeIndex] < reslicePoint && pointBCoords[planeIndex] > reslicePoint) ||
             (pointACoords[planeIndex] > reslicePoint && pointBCoords[planeIndex] < reslicePoint) ||
              areEqual(pointACoords[planeIndex], reslicePoint, planeSpacing) ||
              areEqual(pointBCoords[planeIndex], reslicePoint, planeSpacing))
          {
            if(!newPointIds.contains(pointAId))
            {
              pointACoords[planeIndex] = sliceDepth;
              newPointIds.insert(pointAId, newPoints->InsertNextPoint(pointACoords));
            }

            if(!newPointIds.contains(pointBId))
            {
              pointBCoords[planeIndex] = sliceDepth;
              newPointIds.insert(pointBId, newPoints->InsertNextPoint(pointBCoords));
            }

            auto line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, newPointIds[pointAId]);
            line->GetPointIds()->SetId(1, newPointIds[pointBId]);
            newLines->InsertNextCell(line);
          }
        }

        auto polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(newPoints);
        polyData->SetLines(newLines);

        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);
        mapper->Update();

        auto color = m_colorEngine->color(segmentation);
        double rgba[4];
        s_highlighter.lut(color)->GetTableValue(1,rgba);

        auto actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
        actor->GetProperty()->SetOpacity(opacity(state));

        // TODO: change width if segmentation is/isn't selected. (4 - selected, 2 - not selected);
        actor->GetProperty()->SetLineWidth(4);
        actor->Modified();

        actors << actor;
      }
    }

    return actors;
  }

  //----------------------------------------------------------------------------
  bool SegmentationSkeleton2DPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    // TODO
    return false;
  }

} // namespace ESPINA
