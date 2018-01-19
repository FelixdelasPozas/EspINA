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

// Qt
#include <QDebug>

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
    auto color = m_colorEngine->color(segmentation);
    auto hue   = segmentation->category()->color().hue();
    auto data  = readLockSkeleton(segmentation->output())->skeleton();
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
      line->GetPointIds()->SetId(0, idList->GetId(0));
      line->GetPointIds()->SetId(1, idList->GetId(1));

      if(strokeTypes->GetValue(index) == 0)
      {
        solidColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
        solidCells->InsertNextCell(line);
        solidChanges->InsertNextValue(hue == lineHue ? 0 : 1);
      }
      else
      {
        dashedColors->InsertNextTuple3(rgba[0]*255, rgba[1]*255, rgba[2]*255);
        dashedCells->InsertNextCell(line);
        dashedChanges->InsertNextValue(hue == lineHue ? 0 : 1);
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
    auto edgeNumbers  = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeNumbers"));
    auto strokeNames  = vtkStringArray::SafeDownCast(data->GetPointData()->GetAbstractArray("StrokeName"));

    if(!edgeNumbers || !strokeNames)
    {
      qWarning() << "Bad polydata for" << segmentation->data().toString();
      qWarning() << "Could extract array for edgeNumbers: " << (edgeNumbers == nullptr ? "false" : "true");
      qWarning() << "Could extract array for strokeNames: " << (strokeNames == nullptr ? "false" : "true");
      return actors;
    }

    auto labelPoints = vtkSmartPointer<vtkPoints>::New();
    auto labelText   = vtkSmartPointer<vtkStringArray>::New();
    labelText->SetName("Labels");

    data->GetLines()->InitTraversal();
    for(int i = 0; i < data->GetNumberOfLines(); ++i)
    {
      auto idList = vtkSmartPointer<vtkIdList>::New();
      data->GetLines()->GetNextCell(idList);

      auto index = cellIndexes->GetValue(i);
      auto text = QString(strokeNames->GetValue(edgeIndexes->GetValue(index)).c_str()) + " " + QString::number(edgeNumbers->GetValue(index));
      if(ids.contains(text)) continue;
      ids << text;

      labelPoints->InsertNextPoint(data->GetPoint(idList->GetId(1)));
      labelText->InsertNextValue(text.toStdString().c_str());
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
    labelMapper->SetBackgroundColor(color.redF()*0.6, color.greenF()*0.6, color.blueF()*0.6);
    labelMapper->SetPlaceAllLabels(true);
    labelMapper->SetShapeToRoundedRect();
    labelMapper->SetStyleToFilled();

    auto labelActor = vtkSmartPointer<vtkActor2D>::New();
    labelActor->SetMapper(labelMapper);
    labelActor->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());

    actors << labelActor;
  }

  return actors;
}
