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
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// VTK
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkLabeledDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkTextProperty.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkFollower.h>
#include <vtkFreeTypeLabelRenderStrategy.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

//----------------------------------------------------------------------------
SegmentationSkeleton3DPipeline::SegmentationSkeleton3DPipeline(ColorEngineSPtr colorEngine)
: SegmentationSkeletonPipelineBase{"SegmentationSkeleton3D", colorEngine}
{
  m_truncatedGlyph = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_truncatedGlyph->SetGlyphTypeToSquare();
  m_truncatedGlyph->SetFilled(false);
  m_truncatedGlyph->SetCenter(0,0,0);
  m_truncatedGlyph->SetScale(30);
  m_truncatedGlyph->SetColor(1,0,0);
  m_truncatedGlyph->Update();
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationSkeleton3DPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                               const RepresentationState &state)
{
  auto segmentation = segmentationPtr(item);

  ActorList actors;

  if (segmentation && isVisible(state) && hasSkeletonData(segmentation->output()))
  {
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

    auto data  = readLockSkeleton(segmentation->output(), DataUpdatePolicy::Ignore)->skeleton();
    auto width = segmentation->isSelected() ? 2 : 0;
    width += SegmentationSkeletonPoolSettings::getWidth(state) + 1;

    auto cellIndexes  = vtkIntArray::SafeDownCast(data->GetCellData()->GetAbstractArray("LineIndexes"));
    auto edgeIndexes  = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeIndexes"));
    auto strokeColors = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("StrokeColor"));
    auto strokeTypes  = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("StrokeType"));

    if(!cellIndexes || !strokeColors || !strokeTypes || !edgeIndexes)
    {
      qWarning() << "Bad polydata for" << segmentation->data().toString();
      qWarning() << "Could extract array for cellIndexes: " << (cellIndexes == nullptr ? "false" : "true");
      qWarning() << "Could extract array for edgeIndexes: " << (edgeIndexes == nullptr ? "false" : "true");
      qWarning() << "Could extract array for strokeColors: " << (strokeColors == nullptr ? "false" : "true");
      qWarning() << "Could extract array for strokeTypes: " << (strokeTypes == nullptr ? "false" : "true");

      return actors;
    }

    auto solidColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    solidColors->SetNumberOfComponents(3);

    auto dashedColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    dashedColors->SetNumberOfComponents(3);

    auto solidCells = vtkSmartPointer<vtkCellArray>::New();
    auto dashedCells = vtkSmartPointer<vtkCellArray>::New();

    auto solidChanges = vtkSmartPointer<vtkIntArray>::New();
    solidChanges->SetName("ChangeColor");
    auto dashedChanges = vtkSmartPointer<vtkIntArray>::New();
    dashedChanges->SetName("ChangeColor");

    data->GetLines()->InitTraversal();
    for(int i = 0; i < data->GetNumberOfLines(); ++i)
    {
      auto idList = vtkSmartPointer<vtkIdList>::New();
      data->GetLines()->GetNextCell(idList);

      if(idList->GetNumberOfIds() != 2) continue;

      auto index = edgeIndexes->GetValue(cellIndexes->GetValue(i));
      auto lineHue = strokeColors->GetValue(index);
      double rgba[4];
      auto colorCondition = (hue == lineHue);

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
      line->GetPointIds()->SetId(0, idList->GetId(0));
      line->GetPointIds()->SetId(1, idList->GetId(1));

      if(strokeTypes->GetValue(index) == 0)
      {
        solidColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
        solidCells->InsertNextCell(line);
        solidChanges->InsertNextValue(colorCondition ? 0 : 1);
      }
      else
      {
        dashedColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
        dashedCells->InsertNextCell(line);
        dashedChanges->InsertNextValue(colorCondition ? 0 : 1);
      }
    }

    auto solidData = vtkSmartPointer<vtkPolyData>::New();
    solidData->SetPoints(data->GetPoints());
    solidData->SetLines(solidCells);
    solidData->GetCellData()->SetScalars(solidColors);
    solidData->GetCellData()->AddArray(solidChanges);

    auto dashedData = vtkSmartPointer<vtkPolyData>::New();
    dashedData->SetPoints(data->GetPoints());
    dashedData->SetLines(dashedCells);
    dashedData->GetCellData()->SetScalars(dashedColors);
    dashedData->GetCellData()->AddArray(dashedChanges);

    auto solidMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    solidMapper->SetInputData(solidData);
    solidMapper->Update();

    auto dashedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    dashedMapper->SetInputData(dashedData);
    dashedMapper->Update();

    auto solidActor = vtkSmartPointer<vtkActor>::New();
    solidActor->SetMapper(solidMapper);
    solidActor->GetProperty()->SetLineWidth(width);
    solidActor->Modified();

    auto dashedActor = vtkSmartPointer<vtkActor>::New();
    dashedActor->SetMapper(dashedMapper);
    dashedActor->GetProperty()->SetLineWidth(width);
    dashedActor->GetProperty()->SetLineStipplePattern(0xFF00);
    dashedActor->Modified();

    actors << solidActor << dashedActor;

    QStringList ids;
    auto edgeNumbers   = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeNumbers"));
    auto strokeNames   = vtkStringArray::SafeDownCast(data->GetPointData()->GetAbstractArray("StrokeName"));
    auto edgeTruncated = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeTruncated"));
    auto edgeParents   = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeParents"));

    if(!edgeNumbers || !strokeNames || !edgeParents || !edgeTruncated)
    {
      qWarning() << "Bad polydata for" << segmentation->data().toString();
      qWarning() << "Could extract array for edgeNumbers: " << (edgeNumbers == nullptr ? "false" : "true");
      qWarning() << "Could extract array for strokeNames: " << (strokeNames == nullptr ? "false" : "true");
      qWarning() << "Could extract array for edgeParents: " << (edgeParents == nullptr ? "false" : "true");
      qWarning() << "Could extract array for edgeTruncated: " << (edgeTruncated == nullptr ? "false" : "true");

      return actors;
    }

    auto flags = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("Flags"));
    auto truncatedPoints = vtkSmartPointer<vtkPoints>::New();

    if(flags)
    {
      for(auto i = 0; i < flags->GetNumberOfTuples(); ++i)
      {
        auto nodeFlags = static_cast<SkeletonNodeFlags>(flags->GetValue(i));
        if(nodeFlags.testFlag(SkeletonNodeProperty::TRUNCATED))
        {
          truncatedPoints->InsertNextPoint(data->GetPoint(i));
        }
      }
      truncatedPoints->Modified();
    }

    auto labelPoints = vtkSmartPointer<vtkPoints>::New();
    auto labelText   = vtkSmartPointer<vtkStringArray>::New();
    labelText->SetName("Labels");

    data->GetLines()->InitTraversal();
    QList<NmVector3> usedPoints;
    for(int i = 0; i < data->GetNumberOfLines(); ++i)
    {
      auto idList = vtkSmartPointer<vtkIdList>::New();
      data->GetLines()->GetNextCell(idList);

      auto index  = cellIndexes->GetValue(i);
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
      if(edgeTruncated && edgeTruncated->GetValue(index)) text += QString(" (Truncated)");

      if(ids.contains(text)) continue;
      ids << text;

      double coordsA[3];
      bool inserted = false;
      for(auto j: {1,0})
      {
        data->GetPoint(idList->GetId(j), coordsA);
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
        data->GetPoint(idList->GetId(1), coordsB);
        for(auto j: {0,1,2}) coordsA[j] = (coordsA[j] + coordsB[j])/2.;
      }

      labelPoints->InsertNextPoint(coordsA);
      labelText->InsertNextValue(text.toStdString().c_str());
    }

    auto labelsData = vtkSmartPointer<vtkPolyData>::New();
    labelsData->SetPoints(labelPoints);
    labelsData->GetPointData()->AddArray(labelText);

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

    auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
    labelMapper->SetInputConnection(labelFilter->GetOutputPort());
    labelMapper->SetGeneratePerturbedLabelSpokes(false);
    labelMapper->SetPlaceAllLabels(true);
    labelMapper->SetMaximumLabelFraction(0.9);
    labelMapper->SetUseDepthBuffer(false);
    labelMapper->SetShapeToRoundedRect();
    labelMapper->SetStyleToFilled();
    labelMapper->SetRenderStrategy(strategy);
    labelMapper->SetBackgroundColor(color.redF()*0.6, color.greenF()*0.6, color.blueF()*0.6);
    labelMapper->SetBackgroundOpacity(0.5);
    labelMapper->SetMargin(5);

    auto labelActor = vtkSmartPointer<vtkActor2D>::New();
    labelActor->SetMapper(labelMapper);
    labelActor->SetPickable(false);
    labelActor->SetDragable(false);
    labelActor->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());

    actors << labelActor;

    if(flags && truncatedPoints->GetNumberOfPoints() > 0)
    {
      for(int i = 0; i < truncatedPoints->GetNumberOfPoints(); ++i)
      {
        actors << createTruncatedPointActor(truncatedPoints->GetPoint(i));
      }
    }
  }

  return actors;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkFollower> SegmentationSkeleton3DPipeline::createTruncatedPointActor(const double* point) const
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(1);
  points->SetPoint(0, point);
  points->Modified();

  auto data = vtkSmartPointer<vtkPolyData>::New();
  data->SetPoints(points);
  data->Modified();

  auto mapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  mapper->SetScalarVisibility(false);
  mapper->SetSourceIndexing(false);
  mapper->SetStatic(true);
  mapper->SetInputData(data);
  mapper->SetSourceData(m_truncatedGlyph->GetOutput());
  mapper->Update();

  auto actor = vtkSmartPointer<vtkFollower>::New();
  actor->SetMapper(mapper);
  actor->SetDragable(false);
  actor->SetPickable(false);
  actor->SetOrigin(point);
  actor->GetProperty()->SetColor(1, 0, 0);
  actor->GetProperty()->Modified();
  actor->SetPosition(0,0,0);
  actor->Modified();

  return actor;
}
