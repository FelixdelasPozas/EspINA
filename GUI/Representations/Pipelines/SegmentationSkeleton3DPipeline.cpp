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
#include <vtkTubeFilter.h>
#include <vtkLabeledDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkTextProperty.h>

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
    auto data        = readLockSkeleton(segmentation->output())->skeleton();
    auto width       = SegmentationSkeletonPoolSettings::getWidth(state);
    auto mapperInput = vtkSmartPointer<vtkPolyData>::New();

    if(width == 0)
    {
      mapperInput = data;
    }
    else
    {
      // Create a tube (cylinder) around the line
      auto tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
      tubeFilter->SetInputData(data);
      tubeFilter->SetRadius(width + (item->isSelected() ? 2 : 0));
      tubeFilter->SetNumberOfSides(30);
      tubeFilter->Update();

      mapperInput->DeepCopy(tubeFilter->GetOutput());
    }

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(mapperInput);
    mapper->Update();

    auto color = m_colorEngine->color(segmentation);
    double rgba[4];
    SegmentationSkeletonPipelineBase::s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(rgba[0], rgba[1], rgba[2]);
    actor->GetProperty()->SetLineWidth(width);
    actor->Modified();

    actors << actor;

    if(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected())
    {
      auto labelPoints = vtkSmartPointer<vtkPoints>::New();
      auto labelText   = vtkSmartPointer<vtkStringArray>::New();
      labelText->SetName("Labels");

      auto labels = vtkStringArray::SafeDownCast(data->GetPointData()->GetAbstractArray("Annotations"));

      if(!labels) return actors;

      for(int i = 0; i < labels->GetNumberOfValues(); ++i)
      {
        if(!labels->GetValue(i).empty())
        {
          labelPoints->InsertNextPoint(data->GetPoint(i));
          labelText->InsertNextValue(labels->GetValue(i));
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

  return actors;
}
