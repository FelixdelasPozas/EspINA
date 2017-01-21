/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "vtkZoomSelectionWidgetRepresentation.h"

// vtk
#include <vtkPoints.h>
#include <vtkObjectFactory.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCoordinate.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <QDebug>

using namespace ESPINA::GUI::View::Widgets;

vtkStandardNewMacro(vtkZoomSelectionWidgetRepresentation);

//----------------------------------------------------------------------------
vtkZoomSelectionWidgetRepresentation::vtkZoomSelectionWidgetRepresentation()
: m_type         {vtkZoomSelectionWidget::NONE}
, m_worldPoints  {vtkSmartPointer<vtkPoints>::New()}
, m_lineActor    {vtkSmartPointer<vtkActor2D>::New()}
{
  double point[3] = {-1,-1,-1};
  m_worldPoints->SetNumberOfPoints(5);
  m_worldPoints->SetPoint(0, point);
  m_worldPoints->SetPoint(1, point);
  m_worldPoints->SetPoint(2, point);
  m_worldPoints->SetPoint(3, point);
  m_worldPoints->SetPoint(4, point);

  auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
  polyLine->GetPointIds()->SetNumberOfIds(5);
  for(unsigned int i = 0; i < 5; i++)
  {
    polyLine->GetPointIds()->SetId(i,i);
  }

  auto cells = vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(polyLine);

  // Create a polydata to store everything in
  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(m_worldPoints);
  polyData->SetLines(cells);

  // Setup actor and mapper
  auto mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  mapper->SetInputData(polyData);
  mapper->SetUpdateExtentToWholeExtent();

  m_lineActor->SetMapper(mapper);
  m_lineActor->GetProperty()->SetColor(1,1,1);
  m_lineActor->GetProperty()->SetLineStipplePattern(0XF0F0);
  m_lineActor->GetProperty()->SetLineStippleRepeatFactor(1);
  m_lineActor->GetProperty()->SetLineWidth(2);
}

//----------------------------------------------------------------------------
vtkZoomSelectionWidgetRepresentation::~vtkZoomSelectionWidgetRepresentation()
{
  m_worldPoints = nullptr;
  m_lineActor = nullptr;
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::SetWidgetType(vtkZoomSelectionWidget::WidgetType type)
{
  if (m_type == vtkZoomSelectionWidget::NONE && type != vtkZoomSelectionWidget::NONE)
  {
    m_type = type;
  }
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::BuildRepresentation()
{
  if (this->GetMTime() > this->BuildTime ||
      m_worldPoints->GetMTime() > this->BuildTime ||
      m_lineActor->GetMTime() > this->BuildTime)
    {
      this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------------
int vtkZoomSelectionWidgetRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  return 0;
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::StartWidgetInteraction(double e[2])
{
  double displayPos[3] = { e[0], e[1], 0.0 };
  for(int i = 0; i < m_worldPoints->GetNumberOfPoints(); ++i)
  {
    m_worldPoints->SetPoint(i, displayPos);
  }
  m_worldPoints->Modified();
  m_lineActor->GetMapper()->Update();

  m_lineActor->VisibilityOn();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::WidgetInteraction(double e[2])
{
  double displayPos[3];

  m_worldPoints->GetPoint(1, displayPos);
  displayPos[0] = e[0];
  m_worldPoints->SetPoint(1, displayPos);

  m_worldPoints->GetPoint(2, displayPos);
  displayPos[0] = e[0];
  displayPos[1] = e[1];
  m_worldPoints->SetPoint(2, displayPos);

  m_worldPoints->GetPoint(3, displayPos);
  displayPos[1] = e[1];
  m_worldPoints->SetPoint(3, displayPos);

  m_worldPoints->Modified();
  m_lineActor->GetMapper()->Update();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::EndWidgetInteraction(double e[2])
{
  // compute last point
  WidgetInteraction(e);

  // if the distance between corners is too small, reset and return
  double dPos1[3], dPos2[3];
  m_worldPoints->GetPoint(0, dPos1);
  m_worldPoints->GetPoint(2, dPos2);
  double dist = sqrt((dPos2[0]-dPos1[0])*(dPos2[0]-dPos1[0]) +
                     (dPos2[1]-dPos1[1])*(dPos2[1]-dPos1[1]) +
                     (dPos2[2]-dPos1[2])*(dPos2[2]-dPos1[2]));


  // change camera position
  double point1[3], point2[3], oldCameraPos[3], focalPoint[3];
  this->Renderer->GetActiveCamera()->GetPosition(oldCameraPos);

  m_worldPoints->GetPoint(0, point1);
  m_worldPoints->GetPoint(2, point2);
  point1[0] = (point1[0] + point2[0]) / 2;
  point1[1] = (point1[1] + point2[1]) / 2;
  point1[2] = (point1[2] + point2[2]) / 2;

  double wPoint[4];
  this->Renderer->SetDisplayPoint(point1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(wPoint);

  switch (m_type)
  {
    case vtkZoomSelectionWidget::AXIAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(wPoint[0], wPoint[1], oldCameraPos[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(wPoint);
      break;
    case vtkZoomSelectionWidget::CORONAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(wPoint[0], oldCameraPos[1], wPoint[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(wPoint);
      break;
    case vtkZoomSelectionWidget::SAGITTAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(oldCameraPos[0], wPoint[1], wPoint[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(wPoint);
      break;
    default:
//      if(1 < dist) return; // limits 3D interaction.

      this->Renderer->GetActiveCamera()->GetFocalPoint(focalPoint);
      this->Renderer->GetActiveCamera()->SetPosition(wPoint[0]-(focalPoint[0] - oldCameraPos[0]),
                                                     wPoint[1]-(focalPoint[1] - oldCameraPos[1]),
                                                     wPoint[2]-(focalPoint[2] - oldCameraPos[2]));
      this->Renderer->GetActiveCamera()->SetFocalPoint(wPoint);
      break;
  }

  if (3 < dist)
  {
    // actual zoom over new camera position
    double ll[3], ur[3];
    double factorWidth, factorHeight, zoomFactor;
    vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
    coords->SetViewport(this->Renderer);
    coords->SetCoordinateSystemToNormalizedViewport();
    coords->SetValue(0, 0);
    memcpy(ll,coords->GetComputedDoubleDisplayValue(this->Renderer),3*sizeof(double));
    coords->SetValue(1, 1);
    memcpy(ur,coords->GetComputedDoubleDisplayValue(this->Renderer),3*sizeof(double));
    factorWidth = (fabs(ll[0]-ur[0])/fabs(dPos2[0]-dPos1[0]));
    factorHeight = (fabs(ll[1]-ur[1])/fabs(dPos2[1]-dPos1[1]));
    zoomFactor = (factorHeight > factorWidth) ? factorWidth : factorHeight;
    this->Renderer->GetActiveCamera()->Zoom(zoomFactor);
    this->Renderer->ResetCameraClippingRange();
  }
  else
  {
    // zoom using the viewport coordinates (0.25,0.25)-(0.75, 0.75) for a default zoom
    this->Renderer->GetActiveCamera()->Zoom(2);
    this->Renderer->ResetCameraClippingRange();
  }

  // reset state
  double point[3] = {-1,-1,-1};
  m_worldPoints->SetPoint(0, point);
  m_worldPoints->SetPoint(1, point);
  m_worldPoints->SetPoint(2, point);
  m_worldPoints->SetPoint(3, point);
  m_worldPoints->SetPoint(4, point);

  m_lineActor->VisibilityOff();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  m_lineActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------------
int vtkZoomSelectionWidgetRepresentation::RenderOverlay(vtkViewport *viewport)
{
  this->BuildRepresentation();

  int result = 0;

  if (m_lineActor->GetVisibility())
  {
    result = m_lineActor->RenderOverlay(viewport);
  }

  return result;
}

//----------------------------------------------------------------------------
int vtkZoomSelectionWidgetRepresentation::RenderOpaqueGeometry(vtkViewport *viewport)
{
  this->BuildRepresentation();

  int result = 0;

  if (m_lineActor->GetVisibility())
  {
    result = m_lineActor->RenderOpaqueGeometry(viewport);
  }

  return result;
}
