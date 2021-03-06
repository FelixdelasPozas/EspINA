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
#include <Core/Analysis/Data/SkeletonDataUtils.h>
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
#include <vtkTransformPolyDataFilter.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkTransform.h>
#include <vtkFreeTypeLabelRenderStrategy.h>

// C++
#include <cstring>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Settings;
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
      auto skeleton  = readLockSkeleton(segmentation->output(), DataUpdatePolicy::Ignore)->skeleton();

      auto cellIndexes  = vtkIntArray::SafeDownCast(skeleton->GetCellData()->GetAbstractArray("LineIndexes"));
      auto edgeIndexes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeIndexes"));
      auto strokeColors = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeColor"));
      auto strokeTypes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeType"));
      auto flags        = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("Flags"));

      if(!cellIndexes || !strokeColors || !strokeTypes || !edgeIndexes)
      {
        qWarning() << "Bad polydata for" << segmentation->data().toString();
        qWarning() << "Could extract array for cellIndexes: " << (cellIndexes == nullptr ? "false" : "true");
        qWarning() << "Could extract array for edgeIndexes: " << (edgeIndexes == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeColors: " << (strokeColors == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeTypes: " << (strokeTypes == nullptr ? "false" : "true");
        return actors;
      }

      QColor color;
      if(segmentation->colorEngine() != nullptr)
      {
        color = segmentation->colorEngine()->color(segmentation);
      }
      else
      {
        color = m_colorEngine->color(segmentation);
      }

      auto hue = segmentation->category()->color().hue();

      auto newPoints = vtkSmartPointer<vtkPoints>::New();
      auto truncatedPoints = vtkSmartPointer<vtkPoints>::New();
      int newNum = 0, truncatedNum = 0;

      auto addIfTruncated = [&truncatedNum, flags, skeleton, truncatedPoints](vtkIdType i)
      {
        if(flags)
        {
          auto nodeFlags = static_cast<SkeletonNodeFlags>(flags->GetValue(i));
          if(nodeFlags.testFlag(SkeletonNodeProperty::TRUNCATED))
          {
            ++truncatedNum;
            truncatedPoints->InsertNextPoint(skeleton->GetPoint(i));
          }
        }
      };

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

      auto spacing = segmentation->output()->spacing();

      auto points = skeleton->GetPoints();
      auto lines  = skeleton->GetLines();
      double pointACoords[3]{0, 0, 0};
      double pointBCoords[3]{0, 0, 0};

      auto idList = vtkSmartPointer<vtkIdList>::New();
      auto sliceDepth = reslicePoint + segmentationDepth(state);

      QMap<vtkIdType, vtkIdType> newPointIds;
      QMap<vtkIdType, int> insertedLines;
      int solidNum = 0, dashedNum = 0;

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
            ++newNum;
            newPointIds.insert(pointAId, newPoints->InsertNextPoint(pointACoords));
            insertedLines.insert(pointAId, cellIndexes->GetValue(i));
            addIfTruncated(pointAId);
          }

          if (!newPointIds.contains(pointBId))
          {
            pointBCoords[planeIndex] = sliceDepth;
            ++newNum;
            newPointIds.insert(pointBId, newPoints->InsertNextPoint(pointBCoords));
            insertedLines.insert(pointBId, cellIndexes->GetValue(i));
            addIfTruncated(pointBId);
          }

          auto index = edgeIndexes->GetValue(cellIndexes->GetValue(i));
          auto lineHue = strokeColors->GetValue(index);
          double rgba[4];
          auto colorCondition = (hue == lineHue) || (lineHue == -1);

          if(colorCondition)
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
            ++solidNum;
            solidColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
            solidLines->InsertNextCell(line);
            solidChanges->InsertNextValue(colorCondition ? 0 : 1);
          }
          else
          {
            ++dashedNum;
            dashedColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
            dashedLines->InsertNextCell(line);
            dashedChanges->InsertNextValue(colorCondition ? 0 : 1);
          }
        }
      }

      solidColors->Resize(solidNum);
      solidLines->SetNumberOfCells(solidNum);
      solidChanges->Resize(solidNum);

      dashedColors->Resize(dashedNum);
      dashedLines->SetNumberOfCells(dashedNum);
      dashedChanges->Resize(dashedNum);

      newPoints->Modified();
      newPoints->Resize(newNum);
      newPoints->Squeeze();

      auto solidData = vtkSmartPointer<vtkPolyData>::New();
      solidData->SetPoints(newPoints);
      solidData->SetLines(solidLines);
      solidData->GetCellData()->AddArray(solidChanges);
      solidData->GetCellData()->SetScalars(solidColors);
      solidData->RemoveGhostCells();
      solidData->Modified();

      auto dashedData = vtkSmartPointer<vtkPolyData>::New();
      dashedData->SetPoints(newPoints);
      dashedData->SetLines(dashedLines);
      dashedData->GetCellData()->AddArray(dashedChanges);
      dashedData->GetCellData()->SetScalars(dashedColors);
      dashedData->RemoveGhostCells();
      dashedData->Modified();

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
      dashedActor->GetProperty()->SetColor(1,0,0);

      SegmentationSkeletonPipelineBase::stippledLine(dashedActor, 0xF0F0);

      dashedActor->Modified();

      actors << solidActor << dashedActor;

      QStringList ids;
      auto edgeNumbers   = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeNumbers"));
      auto edgeTruncated = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeTruncated"));
      auto edgeParents   = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeParents"));
      auto strokeNames   = vtkStringArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeName"));

      if(!edgeNumbers || !strokeNames || !edgeParents || !edgeTruncated)
      {
        qWarning() << "Bad polydata for" << segmentation->data().toString();
        qWarning() << "Could extract array for edgeNumbers: " << (edgeNumbers == nullptr ? "false" : "true");
        qWarning() << "Could extract array for strokeNames: " << (strokeNames == nullptr ? "false" : "true");
        qWarning() << "Could extract array for edgeParents: " << (edgeParents == nullptr ? "false" : "true");
        qWarning() << "Could extract array for edgeTruncated: " << (edgeTruncated == nullptr ? "false" : "true");
        return actors;
      }

      // 2020-05-23: fix spine name if child is truncated
      for(auto i = 0; i < edgeTruncated->GetNumberOfTuples(); ++i)
      {
        if(!edgeTruncated->GetValue(i)) continue;
        auto parentIndex = edgeParents->GetValue(i);
        QSet<int> visited;
        while(parentIndex != -1 && !visited.contains(parentIndex))
        {
          auto text = QString(strokeNames->GetValue(edgeIndexes->GetValue(parentIndex)).c_str());
          if(text.startsWith("spine", Qt::CaseInsensitive))
          {
            edgeTruncated->SetValue(parentIndex, edgeTruncated->GetValue(i));
            parentIndex = -1;
          }
          else
          {
            visited << parentIndex;
            parentIndex = edgeParents->GetValue(parentIndex);
          }
        }
      }

      auto labelPoints = vtkSmartPointer<vtkPoints>::New();
      int labelNum = 0;
      auto labelText   = vtkSmartPointer<vtkStringArray>::New();
      labelText->SetName("Labels");

      QList<NmVector3> usedPoints;
      for(auto id: insertedLines.keys())
      {
        auto index  = insertedLines[id];
        auto text   = QString(strokeNames->GetValue(edgeIndexes->GetValue(index)).c_str());
        auto number = QString::number(edgeNumbers->GetValue(index));
        auto otherIndex = edgeParents->GetValue(index);
        QSet<int> visited;
        while((otherIndex != -1) && !visited.contains(otherIndex))
        {
          visited << otherIndex;
          number = QString("%1.%2").arg(edgeNumbers->GetValue(otherIndex)).arg(number);
          otherIndex = edgeParents->GetValue(otherIndex);
        }
        text += QString(" %1").arg(number);
        if(edgeTruncated->GetValue(index)) text += QString(" (Truncated)");

        if(ids.contains(text)) continue;
        ids << text;

        double coordsA[3];
        bool inserted = false;
        const auto linePointsIds = insertedLines.keys(index);
        for(auto j: linePointsIds)
        {
          newPoints->GetPoint(newPointIds[j], coordsA);
          const NmVector3 pointCoords{coordsA[0], coordsA[1], coordsA[2]};
          if(!usedPoints.contains(pointCoords))
          {
            usedPoints << pointCoords;
            inserted = true;
            break;
          }
        }

        if(!inserted)
        {
          // both points are being used. coordsA contains the coordinates of the last point in the line.
          double coordsB[3];
          newPoints->GetPoint(newPointIds[linePointsIds.first()], coordsB);
          for(auto j: {0,1,2}) coordsA[j] = (coordsA[j] + coordsB[j])/2.;
        }

        ++labelNum;
        labelPoints->InsertNextPoint(coordsA);
        labelText->InsertNextValue(text.toStdString().c_str());
      }

      labelPoints->Modified();
      labelPoints->Resize(labelNum);
      labelPoints->Squeeze();

      auto labelsData = vtkSmartPointer<vtkPolyData>::New();
      labelsData->SetPoints(labelPoints);
      labelsData->SetLines(nullptr);
      labelsData->GetPointData()->AddArray(labelText);
      labelsData->RemoveGhostCells();
      labelsData->Modified();

      auto textSize =  SegmentationSkeletonPoolSettings::getAnnotationsSize(state);

      auto property = vtkSmartPointer<vtkTextProperty>::New();
      property->SetBold(true);
      property->SetFontFamilyToArial();
      property->SetFontSize(textSize);
      property->SetJustificationToCentered();
      property->SetVerticalJustificationToCentered();
      property->Modified();

      auto labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
      labelFilter->SetInputData(labelsData);
      labelFilter->SetLabelArrayName("Labels");
      labelFilter->GetTextProperty()->SetFontSize(textSize);
      labelFilter->GetTextProperty()->SetBold(true);
      labelFilter->Update();

      auto strategy = vtkSmartPointer<vtkFreeTypeLabelRenderStrategy>::New();
      strategy->SetDefaultTextProperty(property);

      auto hueColor = QColor::fromHsv(hue, 255,255);
      auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
      labelMapper->SetInputConnection(labelFilter->GetOutputPort());
      labelMapper->SetGeneratePerturbedLabelSpokes(false);
      labelMapper->SetPlaceAllLabels(true);
      labelMapper->SetMaximumLabelFraction(0.9);
      labelMapper->SetUseDepthBuffer(false);
      labelMapper->SetShapeToRoundedRect();
      labelMapper->SetStyleToFilled();
      labelMapper->SetRenderStrategy(strategy);
      labelMapper->SetBackgroundColor(hueColor.redF() * 0.6, hueColor.greenF() * 0.6, hueColor.blueF() * 0.6);
      labelMapper->SetBackgroundOpacity(0.5);
      labelMapper->SetMargin(5);
      labelMapper->Update();

      auto labelActor = vtkSmartPointer<vtkActor2D>::New();
      labelActor->SetMapper(labelMapper);
      labelActor->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());

      actors << labelActor;

      truncatedPoints->Modified();
      truncatedPoints->Resize(truncatedNum);
      truncatedPoints->Squeeze();

      auto truncatedData = vtkSmartPointer<vtkPolyData>::New();
      truncatedData->SetPoints(truncatedPoints);
      truncatedData->SetLines(nullptr);
      truncatedData->RemoveGhostCells();
      truncatedData->Modified();

      double minSpacing = m_plane == Plane::XY ? std::min(spacing[0], spacing[1]) : (m_plane == Plane::XZ ? std::min(spacing[0], spacing[2]) : std::min(spacing[1], spacing[2]));

      auto glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
      glyph2D->SetGlyphTypeToSquare();
      glyph2D->SetFilled(false);
      glyph2D->SetScale(10*minSpacing);
      glyph2D->SetCenter(0,0,0);
      glyph2D->SetColor(1,0,0);
      glyph2D->Update();

      auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

      auto glyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
      glyphMapper->SetScalarVisibility(false);
      glyphMapper->SetStatic(true);
      glyphMapper->SetInputData(truncatedData);

      if(m_plane != Plane::XY)
      {
        auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->RotateWXYZ(90, (m_plane == Plane::YZ ? 0 : 1), (m_plane == Plane::XZ ? 0 : 1), 0);
        transformFilter->SetTransform(transform);
        transformFilter->SetInputData(glyph2D->GetOutput());
        transformFilter->Update();

        glyphMapper->SetSourceData(transformFilter->GetOutput());
      }
      else
      {
        glyphMapper->SetSourceData(glyph2D->GetOutput());
      }

      glyphMapper->Update();

      auto truncatedActor = vtkSmartPointer<vtkActor>::New();
      truncatedActor->SetMapper(glyphMapper);
      truncatedActor->GetProperty()->SetColor(1, 0, 0);
      truncatedActor->GetProperty()->Modified();
      truncatedActor->SetPosition(0, 0, 0);
      truncatedActor->Modified();

      actors << truncatedActor;
    }
  }

  return actors;
}
