/*
 * vtkZoomSelectionWidgetRepresentation.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "vtkZoomSelectionWidgetRepresentation.h"

// vtk
#include <vtkPoints.h>
#include <vtkObjectFactory.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCoordinate.h>

vtkStandardNewMacro(vtkZoomSelectionWidgetRepresentation);

//----------------------------------------------------------------------------
vtkZoomSelectionWidgetRepresentation::vtkZoomSelectionWidgetRepresentation()
: m_type(vtkZoomSelectionWidget::NONE)
, m_displayPoints(vtkSmartPointer<vtkPoints>::New())
, m_worldPoints(vtkSmartPointer<vtkPoints>::New())
, m_lineActor(vtkSmartPointer<vtkActor>::New())
{
  double point[3] = {-1,-1,-1};
  m_displayPoints->SetNumberOfPoints(5);
  m_worldPoints->SetNumberOfPoints(5);
  m_displayPoints->SetPoint(0, point);
  m_displayPoints->SetPoint(1, point);
  m_displayPoints->SetPoint(2, point);
  m_displayPoints->SetPoint(3, point);
  m_displayPoints->SetPoint(4, point);

  vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
  polyLine->GetPointIds()->SetNumberOfIds(5);
  for(unsigned int i = 0; i < 5; i++)
    polyLine->GetPointIds()->SetId(i,i);

  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(polyLine);

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(m_worldPoints);
  polyData->SetLines(cells);

  // Setup actor and mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(polyData->GetProducerPort());

  m_lineActor->SetMapper(mapper);
  m_lineActor->GetProperty()->SetColor(1,1,1);
  m_lineActor->GetProperty()->SetLineStipplePattern(0XF0F0);
  m_lineActor->GetProperty()->SetLineStippleRepeatFactor(1);
  m_lineActor->GetProperty()->SetLineWidth(2);
}

//----------------------------------------------------------------------------
vtkZoomSelectionWidgetRepresentation::~vtkZoomSelectionWidgetRepresentation()
{
  m_displayPoints = NULL;
  m_worldPoints = NULL;
  m_lineActor = NULL;
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::SetWidgetType(vtkZoomSelectionWidget::WidgetType type)
{
  if (m_type != vtkZoomSelectionWidget::NONE || type == vtkZoomSelectionWidget::NONE)
    return;

  m_type = type;

  DisplayPointsToWorldPoints();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::BuildRepresentation()
{
  if (this->GetMTime() > this->BuildTime ||
      m_worldPoints->GetMTime() > this->BuildTime ||
      m_lineActor->GetMTime() > this->BuildTime)
    {
      DisplayPointsToWorldPoints();
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
  for(int i = 0; i < m_displayPoints->GetNumberOfPoints(); ++i)
    m_displayPoints->SetPoint(i, displayPos);

  DisplayPointsToWorldPoints();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::WidgetInteraction(double e[2])
{
  double displayPos[3];

  m_displayPoints->GetPoint(1, displayPos);
  displayPos[0] = e[0];
  m_displayPoints->SetPoint(1, displayPos);

  m_displayPoints->GetPoint(2, displayPos);
  displayPos[0] = e[0];
  displayPos[1] = e[1];
  m_displayPoints->SetPoint(2, displayPos);

  m_displayPoints->GetPoint(3, displayPos);
  displayPos[1] = e[1];
  m_displayPoints->SetPoint(3, displayPos);

  DisplayPointsToWorldPoints();
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::EndWidgetInteraction(double e[2])
{
  // compute last point
  WidgetInteraction(e);

  // if the distance between corners is too small, reset and return
  double dPos1[3], dPos2[3];
  m_displayPoints->GetPoint(0, dPos1);
  m_displayPoints->GetPoint(2, dPos2);
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

  switch (m_type)
  {
    case vtkZoomSelectionWidget::AXIAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(point1[0], point1[1], oldCameraPos[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(point1);
      break;
    case vtkZoomSelectionWidget::CORONAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(point1[0], oldCameraPos[1], point1[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(point1);
      break;
    case vtkZoomSelectionWidget::SAGITTAL_WIDGET:
      this->Renderer->GetActiveCamera()->SetPosition(oldCameraPos[0], point1[1], point1[2]);
      this->Renderer->GetActiveCamera()->SetFocalPoint(point1);
      break;
    default:
      this->Renderer->GetActiveCamera()->GetFocalPoint(focalPoint);
      this->Renderer->GetActiveCamera()->SetPosition(point1[0]-(focalPoint[0] - oldCameraPos[0]),
                                                     point1[1]-(focalPoint[1] - oldCameraPos[1]),
                                                     point1[2]-(focalPoint[2] - oldCameraPos[2]));
      this->Renderer->GetActiveCamera()->SetFocalPoint(point1);
      break;
  }

  if (10 < dist)
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
  m_displayPoints->SetPoint(0, point);
  m_displayPoints->SetPoint(1, point);
  m_displayPoints->SetPoint(2, point);
  m_displayPoints->SetPoint(3, point);
  m_displayPoints->SetPoint(4, point);
  DisplayPointsToWorldPoints();
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
    result = m_lineActor->RenderOverlay(viewport);

  return result;
}

//----------------------------------------------------------------------------
int vtkZoomSelectionWidgetRepresentation::RenderOpaqueGeometry(vtkViewport *viewport)
{
  this->BuildRepresentation();

  int result = 0;

  if (m_lineActor->GetVisibility())
    result = m_lineActor->RenderOpaqueGeometry(viewport);

  return result;
}

//----------------------------------------------------------------------------
void vtkZoomSelectionWidgetRepresentation::DisplayPointsToWorldPoints()
{
  if (!Renderer)
    return;

  double worldPos[4], displayPos[3];

  m_displayPoints->Modified();
  for (int i = 0; i < m_displayPoints->GetNumberOfPoints(); ++i)
  {
    m_displayPoints->GetPoint(i, displayPos);
    this->Renderer->SetDisplayPoint(displayPos);
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(worldPos);

    if (m_type != vtkZoomSelectionWidget::VOLUME_WIDGET)
      worldPos[m_type] = ((vtkZoomSelectionWidget::AXIAL_WIDGET == m_type) ? -0.1 : 0.1);

    m_worldPoints->SetPoint(i, worldPos[0], worldPos[1], worldPos[2]);
  }
  m_worldPoints->Modified();

  m_lineActor->GetMapper()->Update();
}
