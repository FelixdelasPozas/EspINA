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

// Qt
#include <QColor>

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
#include <vtkCellData.h>

// C++
#include <cstring>

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

    if (sliceBounds[2 * planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2 * planeIndex + 1])
    {
      auto skeleton  = readLockSkeleton(segmentation->output())->skeleton();

      auto cellIndexes  = vtkIntArray::SafeDownCast(skeleton->GetCellData()->GetAbstractArray("LineIndexes"));
      auto edgeIndexes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeIndexes"));
      auto strokeColors = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeColor"));
      auto strokeTypes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeType"));

      if(!cellIndexes || !strokeColors || !strokeTypes || !edgeIndexes)
      {
        qWarning() << "Bad polydata for" << segmentation->data().toString();
        qWarning() << "Could extract array for cellIndexes: " << (cellIndexes == nullptr ? "false" : "true");
        qWarning() << "Could extract array for edgeIndexes: " << (edgeIndexes == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeColors: " << (strokeColors == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeTypes: " << (strokeTypes == nullptr ? "false" : "true");

        return actors;
      }

      auto color = m_colorEngine->color(segmentation);
      auto hue   = segmentation->category()->color().hue();

      auto newPoints = vtkSmartPointer<vtkPoints>::New();

      auto solidLines  = vtkSmartPointer<vtkCellArray>::New();
      auto solidColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
      solidColors->SetNumberOfComponents(3);

      auto dashedLines  = vtkSmartPointer<vtkCellArray>::New();
      auto dashedColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
      dashedColors->SetNumberOfComponents(3);

      auto solidChanges = vtkSmartPointer<vtkIntArray>::New();
      solidChanges->SetName("ChangeColor");

      auto dashedChanges = vtkSmartPointer<vtkIntArray>::New();
      dashedChanges->SetName("ChangeColor");

      auto spacing   = segmentation->output()->spacing();

      auto points = skeleton->GetPoints();
      auto lines  = skeleton->GetLines();
      double pointACoords[3]{0, 0, 0};
      double pointBCoords[3]{0, 0, 0};

      auto idList = vtkSmartPointer<vtkIdList>::New();
      auto sliceDepth = reslicePoint + segmentationDepth(state);

      QMap<vtkIdType, vtkIdType> newPointIds;
      QMap<vtkIdType, int> insertedLines;

      lines->InitTraversal();
      for(int i = 0; i < skeleton->GetNumberOfLines(); ++i)
      {
        lines->GetNextCell(idList);

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
            insertedLines.insert(pointAId, cellIndexes->GetValue(i));
          }

          if (!newPointIds.contains(pointBId))
          {
            pointBCoords[planeIndex] = sliceDepth;
            newPointIds.insert(pointBId, newPoints->InsertNextPoint(pointBCoords));
            insertedLines.insert(pointBId, cellIndexes->GetValue(i));
          }

          auto index = edgeIndexes->GetValue(cellIndexes->GetValue(i));
          auto lineHue = strokeColors->GetValue(index);
          double rgba[4];

          if(hue == lineHue)
          {
            s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);
          }
          else
          {
            auto custom = QColor::fromHsv(lineHue, 255, 255);
            s_highlighter.lut(custom, item->isSelected())->GetTableValue(1,rgba);
          }

          auto line = vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, newPointIds[pointAId]);
          line->GetPointIds()->SetId(1, newPointIds[pointBId]);

          if(strokeTypes->GetValue(index) == 0)
          {
            solidColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
            solidLines->InsertNextCell(line);
            solidChanges->InsertNextValue(hue == lineHue ? 0 : 1);
          }
          else
          {
            dashedColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
            dashedLines->InsertNextCell(line);
            dashedChanges->InsertNextValue(hue == lineHue ? 0 : 1);
          }
        }
      }

      auto solidData = vtkSmartPointer<vtkPolyData>::New();
      solidData->SetPoints(newPoints);
      solidData->SetLines(solidLines);
      solidData->GetCellData()->AddArray(solidChanges);
      solidData->GetCellData()->SetScalars(solidColors);

      auto dashedData = vtkSmartPointer<vtkPolyData>::New();
      dashedData->SetPoints(newPoints);
      dashedData->SetLines(dashedLines);
      dashedData->GetCellData()->AddArray(dashedChanges);
      dashedData->GetCellData()->SetScalars(dashedColors);

      auto solidMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      solidMapper->SetInputData(solidData);
      solidMapper->Update();

      auto dashedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      dashedMapper->SetInputData(dashedData);
      dashedMapper->Update();

      auto width = item->isSelected() ? 2 : 1;
      width *= SegmentationSkeletonPoolSettings::getWidth(state) + 1;

      auto solidActor = vtkSmartPointer<vtkActor>::New();
      solidActor->SetMapper(solidMapper);
      solidActor->SetPickable(true);
      solidActor->GetProperty()->SetLineWidth(width);
      solidActor->Modified();

      auto dashedActor = vtkSmartPointer<vtkActor>::New();
      dashedActor->SetMapper(dashedMapper);
      dashedActor->SetPickable(true);
      dashedActor->GetProperty()->SetLineWidth(width);
      dashedActor->GetProperty()->SetLineStipplePattern(0xFF00);
      dashedActor->GetProperty()->SetColor(1,0,0);
      dashedActor->Modified();

      actors << solidActor << dashedActor;

      QStringList ids;
      auto edgeNumbers = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeNumbers"));
      auto strokeNames = vtkStringArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeName"));

      if(!edgeNumbers || !strokeNames)
      {
        qWarning() << "Bad polydata for" << segmentation->data().toString();
        qWarning() << "Could extract array for edgeNumbers: " << (edgeNumbers == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeNames: " << (strokeNames == nullptr ? "false" : "true");
        return actors;
      }

      auto labelPointsBlue = vtkSmartPointer<vtkPoints>::New();
      auto labelTextBlue   = vtkSmartPointer<vtkStringArray>::New();
      labelTextBlue->SetName("Labels");
      auto labelPointsRed = vtkSmartPointer<vtkPoints>::New();
      auto labelTextRed   = vtkSmartPointer<vtkStringArray>::New();
      labelTextRed->SetName("Labels");
      auto labelPointsGreen = vtkSmartPointer<vtkPoints>::New();
      auto labelTextGreen   = vtkSmartPointer<vtkStringArray>::New();
      labelTextGreen->SetName("Labels");

      for(auto id: insertedLines.keys())
      {
        auto index = insertedLines[id];
        auto edgeIndex = edgeIndexes->GetValue(index);
        auto number = edgeNumbers->GetValue(index);
        auto text = QString("%1 %2").arg(strokeNames->GetValue(edgeIndex).c_str()).arg(number);

        if(ids.contains(text)) continue;

        ids << text;

        auto sliceValue = skeleton->GetPoint(id)[planeIndex];
        if(areEqual(sliceValue, reslicePoint))
        {
          labelPointsGreen->InsertNextPoint(newPoints->GetPoint(newPointIds[id]));
          labelTextGreen->InsertNextValue(text.toStdString().c_str());
        }
        else
        {
          if(sliceValue < reslicePoint)
          {
            labelPointsBlue->InsertNextPoint(newPoints->GetPoint(newPointIds[id]));
            labelTextBlue->InsertNextValue(text.toStdString().c_str());
          }
          else
          {
            labelPointsRed->InsertNextPoint(newPoints->GetPoint(newPointIds[id]));
            labelTextRed->InsertNextValue(text.toStdString().c_str());
          }
        }
      }

      auto labelsDataGreen = vtkSmartPointer<vtkPolyData>::New();
      labelsDataGreen->SetPoints(labelPointsGreen);
      labelsDataGreen->GetPointData()->AddArray(labelTextGreen);

      auto labelsDataBlue = vtkSmartPointer<vtkPolyData>::New();
      labelsDataBlue->SetPoints(labelPointsBlue);
      labelsDataBlue->GetPointData()->AddArray(labelTextBlue);

      auto labelsDataRed = vtkSmartPointer<vtkPolyData>::New();
      labelsDataRed->SetPoints(labelPointsRed);
      labelsDataRed->GetPointData()->AddArray(labelTextRed);

      auto property = vtkSmartPointer<vtkTextProperty>::New();
      property->SetBold(true);
      property->SetFontFamilyToArial();
      property->SetFontSize(SegmentationSkeletonPoolSettings::getAnnotationsSize(state));
      property->SetJustificationToCentered();

      for(auto input: {labelsDataGreen, labelsDataBlue, labelsDataRed})
      {
        auto labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
        labelFilter->SetInputData(input);
        labelFilter->SetLabelArrayName("Labels");
        labelFilter->SetTextProperty(property);
        labelFilter->Update();

        double labelColor[3]{(input == labelsDataRed ? 1. : 0.), (input == labelsDataGreen ? 1. : 0.), (input == labelsDataBlue ? 1. : 0.)};

        auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
        labelMapper->SetInputConnection(labelFilter->GetOutputPort());
        labelMapper->SetGeneratePerturbedLabelSpokes(true);
        labelMapper->SetBackgroundColor(labelColor[0] * 0.6, labelColor[1] * 0.6, labelColor[2] * 0.6);
        labelMapper->SetPlaceAllLabels(true);
        labelMapper->SetShapeToRoundedRect();
        labelMapper->SetStyleToFilled();

        auto labelActor = vtkSmartPointer<vtkActor2D>::New();
        labelActor->SetMapper(labelMapper);
        labelActor->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());

        actors << labelActor;
      }
    }
  }

  return actors;
}
