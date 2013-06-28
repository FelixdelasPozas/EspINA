/*
 * vtkPlanarSplitRepresentation2D.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "vtkPlanarSplitRepresentation2D.h"

// Qt
#include <QtGlobal>
#include <QDebug>

// vtk
#include <vtkPoints.h>
#include <vtkObjectFactory.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkHandleRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkRenderer.h>
#include <vtkProperty2D.h>

using namespace EspINA;

vtkStandardNewMacro(vtkPlanarSplitRepresentation2D);

//----------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkPlanarSplitRepresentation2D,HandleRepresentation,vtkHandleRepresentation);

vtkPlanarSplitRepresentation2D::vtkPlanarSplitRepresentation2D()
{
  m_plane = AXIAL;
  m_epsilon = -0.1;
  m_tolerance = 15;
  m_point1[0] = m_point1[1] = m_point1[2] = 0;
  m_point2[0] = m_point2[1] = m_point2[2] = 0;

  m_line = vtkSmartPointer<vtkLineSource>::New();
  m_line->Update();
  m_lineActor = vtkSmartPointer<vtkActor>::New();
  m_boundsPoints = NULL;
  m_boundsActor = NULL;

  HandleRepresentation = vtkPointHandleRepresentation2D::New();
  vtkProperty2D *property = reinterpret_cast<vtkPointHandleRepresentation2D*>(HandleRepresentation)->GetProperty();
  property->SetColor(0,1,0);
  property->SetLineWidth(2);

  Point1Representation = NULL;
  Point2Representation = NULL;

  this->InteractionState = Outside;
}

//----------------------------------------------------------------------
vtkPlanarSplitRepresentation2D::~vtkPlanarSplitRepresentation2D()
{
  if (this->HandleRepresentation)
    this->HandleRepresentation->Delete();

  if (this->Point1Representation)
    this->Point1Representation->Delete();

  if (this->Point2Representation)
    this->Point2Representation->Delete();

  if (m_boundsActor != NULL)
    this->Renderer->RemoveActor(m_boundsActor);
}

//----------------------------------------------------------------------
vtkSmartPointer<vtkPoints> vtkPlanarSplitRepresentation2D::getPoints()
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(2);
  points->SetPoint(0, m_point1);
  points->SetPoint(1, m_point2);

  return points;
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::setPoints(vtkSmartPointer<vtkPoints> points)
{
  if (points->GetNumberOfPoints() == 0)
    return;

  points->GetPoint(0, m_point1);
  points->GetPoint(0, m_point2);

  if (points->GetNumberOfPoints() == 2)
    points->GetPoint(1, m_point2);

  m_point1[m_plane] = m_epsilon;
  m_point2[m_plane] = m_epsilon;

  this->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::setPoint1(Nm *point)
{
  double displayPos[3] = { point[0], point[1], point[2] };
  double worldPos[3];

  this->Renderer->SetDisplayPoint(displayPos);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(worldPos);

  m_point1[0] = worldPos[0];
  m_point1[1] = worldPos[1];
  m_point1[2] = worldPos[2];
  m_point1[m_plane] = m_epsilon;
  m_point2[0] = worldPos[0];
  m_point2[1] = worldPos[1];
  m_point2[2] = worldPos[2];
  m_point2[m_plane] = m_epsilon;


  this->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::setPoint2(Nm *point)
{
  double worldPos[3];

  this->Renderer->SetDisplayPoint(point);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(worldPos);

  m_point2[0] = worldPos[0];
  m_point2[1] = worldPos[1];
  m_point2[2] = worldPos[2];
  m_point2[m_plane] = m_epsilon;

  this->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::getPoint1(Nm *point)
{
  point[0] = m_point1[0];
  point[1] = m_point1[1];
  point[2] = m_point1[2];
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::getPoint2(Nm *point)
{
  point[0] = m_point2[0];
  point[1] = m_point2[1];
  point[2] = m_point2[2];
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::BuildRepresentation()
{
  if (this->GetMTime() > this->BuildTime ||
      m_line->GetMTime() > this->BuildTime ||
      m_lineActor->GetMTime() > this->BuildTime ||
      Point1Representation->GetMTime() > this->BuildTime ||
      Point2Representation->GetMTime() > this->BuildTime)
  {
    m_line->SetPoint1(m_point1[0], m_point1[1], m_point1[2]);
    m_line->SetPoint2(m_point2[0], m_point2[1], m_point2[2]);
    m_line->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(m_line->GetOutputPort());

    m_lineActor->SetMapper(mapper);
    m_lineActor->GetProperty()->SetColor(1,1,1);
    m_lineActor->GetProperty()->SetLineWidth(2);

    m_point1[m_plane] = m_epsilon;
    m_point2[m_plane] = m_epsilon;
    Point1Representation->SetWorldPosition(m_point1);
    Point1Representation->SetTolerance(m_tolerance);
    Point2Representation->SetWorldPosition(m_point2);
    Point2Representation->SetTolerance(m_tolerance);

    this->BuildTime.Modified();
  }
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::ReleaseGraphicsResources(vtkWindow *w)
{
  m_lineActor->ReleaseGraphicsResources(w);
  Point1Representation->ReleaseGraphicsResources(w);
  Point2Representation->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int vtkPlanarSplitRepresentation2D::RenderOverlay(vtkViewport *v)
{
  this->BuildRepresentation();

  int result = 0;

  if (m_lineActor->GetVisibility())
    result = m_lineActor->RenderOverlay(v);

  if (Point1Representation->GetVisibility())
    result |= Point1Representation->RenderOverlay(v);

  if (Point2Representation->GetVisibility())
    result |= Point2Representation->RenderOverlay(v);

  return result;
}

//----------------------------------------------------------------------
int vtkPlanarSplitRepresentation2D::RenderOpaqueGeometry(vtkViewport *v)
{
  this->BuildRepresentation();

  int result = 0;

  if (m_lineActor->GetVisibility())
    result = m_lineActor->RenderOpaqueGeometry(v);

  if (Point1Representation->GetVisibility())
    result |= Point1Representation->RenderOpaqueGeometry(v);

  if (Point2Representation->GetVisibility())
    result |= Point2Representation->RenderOpaqueGeometry(v);

  return result;
}

//----------------------------------------------------------------------
int vtkPlanarSplitRepresentation2D::ComputeInteractionState(int vtkNotUsed(X), int vtkNotUsed(Y), int vtkNotUsed(modify))
{
  if (this->Point1Representation == NULL || this->Point2Representation == NULL)
  {
    this->InteractionState = Outside;
    return this->InteractionState;
  }

  int handle1State = this->Point1Representation->GetInteractionState();
  int handle2State = this->Point2Representation->GetInteractionState();

  if (handle1State == vtkHandleRepresentation::Nearby)
    this->InteractionState = NearP1;
  else
    if (handle2State == vtkHandleRepresentation::Nearby)
      this->InteractionState = NearP2;
    else
      this->InteractionState = Outside;

  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::StartWidgetInteraction(double e[2])
{
  double displayPos[3];
  displayPos[0] = e[0];
  displayPos[1] = e[1];
  displayPos[2] = 0.0;

  this->setPoint1(displayPos);
  this->setPoint2(displayPos);
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::WidgetInteraction(double e[2])
{
  double displayPos[3];
  displayPos[0] = e[0];
  displayPos[1] = e[1];
  displayPos[2] = 0.0;

  this->setPoint2(displayPos);
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::InstantiateHandleRepresentation()
{
  if (!this->Point1Representation)
  {
    this->Point1Representation = this->HandleRepresentation->NewInstance();
    this->Point1Representation->ShallowCopy(this->HandleRepresentation);
    this->Point1Representation->SetDragable(true);
  }

  if (!this->Point2Representation)
  {
    this->Point2Representation = this->HandleRepresentation->NewInstance();
    this->Point2Representation->ShallowCopy(this->HandleRepresentation);
    this->Point2Representation->SetDragable(true);
  }
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::setOrientation(PlaneType plane)
{
  m_plane = plane;
  m_epsilon = ((AXIAL == m_plane) ? -0.1 : 0.1);
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::MoveHandle(int handleNum, int X, int Y)
{
  double displayPos[3] = { static_cast<double>(X), static_cast<double>(Y), 0.0 };
  double worldPos[3];

  this->Renderer->SetDisplayPoint(displayPos);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(worldPos);

  switch(handleNum)
  {
    case 0:
      m_point1[0] = worldPos[0];
      m_point1[1] = worldPos[1];
      m_point1[2] = worldPos[2];
      m_point1[m_plane] = m_epsilon;
      break;
    case 1:
      m_point2[0] = worldPos[0];
      m_point2[1] = worldPos[1];
      m_point2[2] = worldPos[2];
      m_point2[m_plane] = m_epsilon;
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  this->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::setSegmentationBounds(double *bounds)
{
  m_boundsPoints = vtkSmartPointer<vtkPoints>::New();
  m_boundsPoints->SetNumberOfPoints(4);
  double point[3];

  switch (this->m_plane)
  {
    case AXIAL:
      point[0] = bounds[0];
      point[1] = bounds[2];
      point[2] = m_epsilon;
      m_boundsPoints->InsertPoint(0, point);
      point[0] = bounds[0];
      point[1] = bounds[3];
      point[2] = m_epsilon;
      m_boundsPoints->InsertPoint(1, point);
      point[0] = bounds[1];
      point[1] = bounds[3];
      point[2] = m_epsilon;
      m_boundsPoints->InsertPoint(2, point);
      point[0] = bounds[1];
      point[1] = bounds[2];
      point[2] = m_epsilon;
      m_boundsPoints->InsertPoint(3, point);
      break;
    case CORONAL:
      point[0] = bounds[0];
      point[1] = m_epsilon;
      point[2] = bounds[4];
      m_boundsPoints->InsertPoint(0, point);
      point[0] = bounds[0];
      point[1] = m_epsilon;
      point[2] = bounds[5];
      m_boundsPoints->InsertPoint(1, point);
      point[0] = bounds[1];
      point[1] = m_epsilon;
      point[2] = bounds[5];
      m_boundsPoints->InsertPoint(2, point);
      point[0] = bounds[1];
      point[1] = m_epsilon;
      point[2] = bounds[4];
      m_boundsPoints->InsertPoint(3, point);
      break;
    case SAGITTAL:
      point[0] = m_epsilon;
      point[1] = bounds[2];
      point[2] = bounds[4];
      m_boundsPoints->InsertPoint(0, point);
      point[0] = m_epsilon;
      point[1] = bounds[2];
      point[2] = bounds[5];
      m_boundsPoints->InsertPoint(1, point);
      point[0] = m_epsilon;
      point[1] = bounds[3];
      point[2] = bounds[5];
      m_boundsPoints->InsertPoint(2, point);
      point[0] = m_epsilon;
      point[1] = bounds[3];
      point[2] = bounds[4];
      m_boundsPoints->InsertPoint(3, point);
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  m_boundsPoints->Modified();

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->Allocate();
  polyData->SetPoints(m_boundsPoints);
  vtkIdType connectivity[2];
  for (int i = 0; i < 4; ++i)
  {
    connectivity[0] = i;
    connectivity[1] = (i+1)%4;
    polyData->InsertNextCell(VTK_LINE, 2, connectivity);
  }
  polyData->Update();

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(polyData->GetProducerPort());

  m_boundsActor = vtkSmartPointer<vtkActor>::New();
  m_boundsActor->SetMapper(mapper);
  m_boundsActor->GetProperty()->SetColor(1,1,1);
  m_boundsActor->GetProperty()->SetLineStipplePattern(0xFFF0);
  m_boundsActor->GetProperty()->SetLineStippleRepeatFactor(1);
  m_boundsActor->GetProperty()->SetLineWidth(2);

  this->Renderer->AddActor(m_boundsActor);
}

//----------------------------------------------------------------------
void vtkPlanarSplitRepresentation2D::removeBoundsActor()
{
  if (m_boundsActor != NULL)
  {
    this->Renderer->RemoveActor(m_boundsActor);
    m_boundsActor = NULL;
  }
}
