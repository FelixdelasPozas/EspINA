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
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/Utils/RepresentationUtils.h>
#include <Support/Representations/RepresentationUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

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
#include <vtkTextProperty.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkActor2D.h>
#include <vtkPointData.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::RepresentationUtils;

//----------------------------------------------------------------------------
SegmentationSkeleton2DPipeline::SegmentationSkeleton2DPipeline(Plane plane, ColorEngineSPtr colorEngine)
: SegmentationSkeletonPipelineBase{"SegmentationSkeleton2D", colorEngine}
, m_plane                         {plane}
{
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSkeleton2DPipeline::createActors(ConstViewItemAdapterPtr item, const RepresentationState &state)
{
  auto segmentation = segmentationPtr(item);
  auto planeIndex = normalCoordinateIndex(m_plane);

  ActorList actors;
  
  if (segmentation && isVisible(state) && hasSkeletonData(segmentation->output()))
  {
    Bounds sliceBounds = segmentation->bounds();

    Nm reslicePoint = crosshairPosition(m_plane, state);
    
    QMap<vtkIdType, vtkIdType> newPointIds;

    if (sliceBounds[2 * planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2 * planeIndex + 1])
    {
      auto newPoints = vtkSmartPointer<vtkPoints>::New();
      auto newLines  = vtkSmartPointer<vtkCellArray>::New();
      auto skeleton  = readLockSkeleton(segmentation->output())->skeleton();
      auto spacing   = segmentation->output()->spacing();

      auto points = skeleton->GetPoints();
      auto lines = skeleton->GetLines();
      double pointACoords[3]{0, 0, 0};
      double pointBCoords[3]{0, 0, 0};
      auto showIds = SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected();

      lines->InitTraversal();
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      auto sliceDepth = reslicePoint + segmentationDepth(state);
      while (lines->GetNextCell(idList))
      {
        if (idList->GetNumberOfIds() != 2) continue;

        vtkIdType pointAId = idList->GetId(0);
        vtkIdType pointBId = idList->GetId(1);
        points->GetPoint(pointAId, pointACoords);
        points->GetPoint(pointBId, pointBCoords);

        if ((pointACoords[planeIndex] < reslicePoint && pointBCoords[planeIndex] > reslicePoint) ||
            (pointACoords[planeIndex] > reslicePoint && pointBCoords[planeIndex] < reslicePoint) ||
            areEqual(pointACoords[planeIndex], reslicePoint, spacing[planeIndex])                ||
            areEqual(pointBCoords[planeIndex], reslicePoint, spacing[planeIndex]))
        {
          if (!newPointIds.contains(pointAId))
          {
            pointACoords[planeIndex] = sliceDepth;
            newPointIds.insert(pointAId, newPoints->InsertNextPoint(pointACoords));
          }

          if (!newPointIds.contains(pointBId))
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
      SegmentationSkeletonPipelineBase::s_highlighter.lut(color, item->isSelected())->GetTableValue(1, rgba);

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);
      actor->SetPickable(true);
      actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);

      auto width = item->isSelected() ? 2 : 1;
      width *= SegmentationSkeletonPoolSettings::getWidth(state);

      actor->GetProperty()->SetLineWidth(width);
      actor->Modified();

      actors << actor;

      if(showIds)
      {
        auto labelPoints = vtkSmartPointer<vtkPoints>::New();
        auto labelText   = vtkSmartPointer<vtkStringArray>::New();
        labelText->SetName("Labels");

        auto labels = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetScalars("Connections"));

        int number = 0;
        auto usedPointIds = newPointIds.keys();
        for(int i = 0; i < labels->GetNumberOfTuples(); ++i)
        {
          if(labels->GetValue(i) != 2)
          {
            ++number;
            if(usedPointIds.contains(i))
            {
              labelPoints->InsertNextPoint(newPoints->GetPoint(newPointIds[i]));
              labelText->InsertNextValue(QString::number(number).toStdString().c_str());
            }
          }
        }

        auto labelsData = vtkSmartPointer<vtkPolyData>::New();
        labelsData->SetPoints(labelPoints);
        labelsData->GetPointData()->AddArray(labelText);

        auto property = vtkSmartPointer<vtkTextProperty>::New();
        property->SetBold(true);
        property->SetFontFamilyToArial();
        property->SetFontSize(15);
        property->SetJustificationToCentered();

        auto labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
        labelFilter->SetInputData(labelsData);
        labelFilter->SetLabelArrayName("Labels");
        labelFilter->SetTextProperty(property);
        labelFilter->Update();

        auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
        labelMapper->SetInputConnection(labelFilter->GetOutputPort());
        labelMapper->SetGeneratePerturbedLabelSpokes(true);
        labelMapper->SetBackgroundColor(rgba[0]*0.6, rgba[1]*0.6, rgba[2]*0.6);
        labelMapper->SetPlaceAllLabels(true);
        labelMapper->SetShapeToRoundedRect();
        labelMapper->SetStyleToFilled();

        auto labelActor = vtkSmartPointer<vtkActor2D>::New();
        labelActor->SetMapper(labelMapper);

        actors << labelActor;
      }
    }
  }

  return actors;
}
