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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <ToolGroups/Visualize/Representations/ChannelLines/ChannelLinesPipeline.h>
#include <ToolGroups/Visualize/Representations/RepresentationSettings.h>
#include <GUI/Model/ChannelAdapter.h>

// VTK
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  ChannelLinesPipeline::ChannelLinesPipeline()
  : RepresentationPipeline("ChannelLinesRepresentation")
  {
  }

  //----------------------------------------------------------------------------
  RepresentationState ChannelLinesPipeline::representationState(const ViewItemAdapter *item,
                                                                const RepresentationState &settings)
  {
    RepresentationState state;

    auto channel = channelPtr(item);

    state.setValue<double>(Representations::VISIBLE, channel->isVisible());

    state.apply(settings);

    return state;
  }

  //----------------------------------------------------------------------------
  RepresentationPipeline::ActorList ChannelLinesPipeline::createActors(const ViewItemAdapter *item,
                                                                       const RepresentationState &state)
  {
    auto channel = channelPtr(item);

    ActorList actors;
    if (isVisible(state))
    {
      auto volume        = volumetricData(channel->output());
      auto channelBounds = volume->bounds();

      for(auto planeIndex: {0,1,2})
      {
        auto reslicePoint = crosshairPosition(toPlane(planeIndex), state);

        if (channelBounds[2*planeIndex] <= reslicePoint && reslicePoint < channelBounds[2*planeIndex+1])
        {
          auto sliceBounds = channelBounds;
          sliceBounds.setLowerInclusion(true);
          sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
          sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;
  
          double cp0[3] = { sliceBounds[0], sliceBounds[2], sliceBounds[4] };
          double cp1[3] = { sliceBounds[1], sliceBounds[2], sliceBounds[4] };
          double cp2[3] = { sliceBounds[1], sliceBounds[3], sliceBounds[4] };
          double cp3[3] = { sliceBounds[1], sliceBounds[3], sliceBounds[5] };
          double cp4[3] = { sliceBounds[0], sliceBounds[3], sliceBounds[5] };
          double cp5[3] = { sliceBounds[0], sliceBounds[2], sliceBounds[5] };

          auto points = vtkSmartPointer<vtkPoints>::New();
          points->InsertNextPoint(cp0);
          points->InsertNextPoint(cp1);
          points->InsertNextPoint(cp2);
          points->InsertNextPoint(cp3);
          points->InsertNextPoint(cp4);
          points->InsertNextPoint(cp5);
          points->InsertNextPoint(cp0);

          auto borderLines = vtkSmartPointer<vtkCellArray>::New();
          for (unsigned int i = 0; i < 6; i++)
          {
            auto line = vtkLine::New();
            line->GetPointIds()->SetId(0, i);
            line->GetPointIds()->SetId(1, i + 1);
            borderLines->InsertNextCell(line);
            line->Delete();
          }

          auto borderData = vtkSmartPointer<vtkPolyData>::New();
          borderData->SetPoints(points);
          borderData->SetLines(borderLines);
          borderData->Modified();

          auto borderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
          borderMapper->SetInputData(borderData);

          // these colors have been defined in DefaultEspinaView.cpp for the 2D crosshair
          double colors[3][3]{ { 0, 1, 1 }, { 0, 0, 1 },  { 1, 0, 1 } };

          auto borderActor = vtkSmartPointer<vtkActor>::New();
          borderActor->SetMapper(borderMapper);
          borderActor->GetProperty()->SetColor(colors[planeIndex]);
          borderActor->GetProperty()->SetPointSize(2);
          borderActor->GetProperty()->SetLineWidth(1);
          borderActor->SetPickable(false);
          borderActor->Modified();

          actors << borderActor;
        }
      }
    }

    return actors;
   }

  //----------------------------------------------------------------------------
  bool ChannelLinesPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    return false;
  }

} // namespace ESPINA
