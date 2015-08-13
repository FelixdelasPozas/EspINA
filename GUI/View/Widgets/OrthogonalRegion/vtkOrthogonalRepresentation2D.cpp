/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "vtkOrthogonalRepresentation2D.h"

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkInteractorObserver.h>
#include <vtkAssemblyPath.h>
#include <vtkWindow.h>
#include <vtkCellData.h>
#include <vtkPoints.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

vtkStandardNewMacro(vtkOrthogonalRepresentation2D);

//----------------------------------------------------------------------------
vtkOrthogonalRepresentation2D::vtkOrthogonalRepresentation2D()
: Vertex              {nullptr}
, EdgePicker          {nullptr}
, LastPicker          {nullptr}
, CurrentEdge         {nullptr}
, EdgeProperty        {nullptr}
, SelectedEdgeProperty{nullptr}
, InvisibleProperty   {nullptr}
, m_plane             {Plane::UNDEFINED}
, m_slice             {0}
, NumPoints           {4}
, NumSlices           {1}
, NumVertex           {0}
, LeftEdge            {0}
, TopEdge             {0}
, RightEdge           {1}
, BottomEdge          {1}
, m_depth             {0}
, m_pattern           {0xFFFF}
{
  // The initial state
  InteractionState = vtkOrthogonalRepresentation2D::Outside;

  memset(m_bounds, 0, sizeof(double)*6);
  memset(m_repBounds, 0, sizeof(double)*6);

  // default representation color
  m_color[0] = m_color[1] = 1.0;
  m_color[2] = 0.0;

  CreateDefaultProperties();

  //Manage the picking stuff
  EdgePicker = vtkSmartPointer<vtkCellPicker>::New();
  EdgePicker->SetTolerance(0.01);
  EdgePicker->PickFromListOn();

  // Build edges
  Vertex = vtkSmartPointer<vtkPoints>::New();
  Vertex->SetDataTypeToDouble();
  Vertex->SetNumberOfPoints(4);//line sides;
  for (EDGE i=LEFT; i<=BOTTOM; i = EDGE(i+1))
  {
    EdgePolyData[i] = vtkSmartPointer<vtkPolyData>::New();
    EdgeMapper[i]   = vtkSmartPointer<vtkPolyDataMapper>::New();
    EdgeActor[i]    = vtkSmartPointer<vtkActor>::New();

    EdgePolyData[i]->SetPoints(Vertex);
    EdgePolyData[i]->SetLines(vtkSmartPointer<vtkCellArray>::New());
    EdgeMapper[i]->SetInputData(EdgePolyData[i]);
    EdgeActor[i]->SetMapper(EdgeMapper[i]);
    EdgeActor[i]->SetProperty(EdgeProperty);

    EdgePicker->AddPickList(EdgeActor[i]);
  }

  // Define the point coordinates
  double bounds[6] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  PlaceWidget(bounds);

  CurrentEdge = nullptr;
}

//----------------------------------------------------------------------------
vtkOrthogonalRepresentation2D::~vtkOrthogonalRepresentation2D()
{
  for(int i=0; i<4; i++)
  {
    EdgeActor[i] = nullptr;
    EdgeMapper[i] = nullptr;
    EdgePolyData[i] = nullptr;
  }

  EdgePicker = nullptr;
  EdgeProperty = nullptr;
  SelectedEdgeProperty = nullptr;
  InvisibleProperty = nullptr;
}

//----------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::reset()
{
  CreateRegion();
}

//----------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::StartWidgetInteraction(double e[2])
{
  // Store the start position
  StartEventPosition[0] = e[0];
  StartEventPosition[1] = e[1];
  StartEventPosition[2] = 0.0;

  // Store the start position
  LastEventPosition[0] = e[0];
  LastEventPosition[1] = e[1];
  LastEventPosition[2] = 0.0;

  ComputeInteractionState(static_cast<int>(e[0]),static_cast<int>(e[1]),0);
}

//----------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::WidgetInteraction(double e[2])
{
  // Convert events to appropriate coordinate systems
  vtkCamera *camera = Renderer->GetActiveCamera();
  if ( !camera )
    return;

  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z, vpn[3];
  camera->GetViewPlaneNormal(vpn);

  // Compute the two points defining the motion vector
  double pos[3];
  if ( LastPicker == EdgePicker )
  {
    EdgePicker->GetPickPosition(pos);
  }
  vtkInteractorObserver::ComputeWorldToDisplay(Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(Renderer,LastEventPosition[0],
                                               LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( InteractionState == vtkOrthogonalRepresentation2D::MoveLeft )
  {
    MoveLeftEdge(prevPickPoint,pickPoint);
  }

  else if ( InteractionState == vtkOrthogonalRepresentation2D::MoveRight )
  {
    MoveRightEdge(prevPickPoint,pickPoint);
  }

  else if ( InteractionState == vtkOrthogonalRepresentation2D::MoveTop )
  {
    MoveTopEdge(prevPickPoint,pickPoint);
  }

  else if ( InteractionState == vtkOrthogonalRepresentation2D::MoveBottom )
  {
    MoveBottomEdge(prevPickPoint,pickPoint);
  }
  else if ( InteractionState == vtkOrthogonalRepresentation2D::Translating )
  {
    Translate(prevPickPoint,pickPoint);
  }

  // Store the start position
  LastEventPosition[0] = e[0];
  LastEventPosition[1] = e[1];
  LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  LeftEdge     += shift;
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  RightEdge   += shift;
  UpdateRegion();
}
//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  TopEdge     += shift;
  UpdateRegion();
}
//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  BottomEdge  += shift;
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::Translate(double* p1, double* p2)
{
  double hShift = p2[hCoord()] - p1[hCoord()];
  double vShift = p2[vCoord()] - p1[vCoord()];

  LeftEdge   += hShift;
  RightEdge  += hShift;
  TopEdge    += vShift;
  BottomEdge += vShift;

  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::CreateDefaultProperties()
{
  // Edge properties
  EdgeProperty = vtkSmartPointer<vtkProperty>::New();
  EdgeProperty->SetRepresentationToSurface();
  EdgeProperty->SetOpacity(1.0);
  EdgeProperty->SetColor(m_color);
  EdgeProperty->SetLineWidth(1.0);
  EdgeProperty->SetLineStipplePattern(m_pattern);

  // Selected Edge properties
  SelectedEdgeProperty = vtkSmartPointer<vtkProperty>::New();
  SelectedEdgeProperty->SetRepresentationToSurface();
  SelectedEdgeProperty->SetOpacity(1.0);
  SelectedEdgeProperty->SetColor(m_color);
  SelectedEdgeProperty->SetLineWidth(2.0);
  SelectedEdgeProperty->SetLineStipplePattern(m_pattern);

  InvisibleProperty = vtkSmartPointer<vtkProperty>::New();
  InvisibleProperty->SetRepresentationToWireframe();
  InvisibleProperty->SetAmbient(0.0);
  InvisibleProperty->SetDiffuse(0.0);
  InvisibleProperty->SetOpacity(0);
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::CreateRegion()
{
  // Corners of the rectangular region
  Vertex->SetNumberOfPoints(4);

  for(EDGE i=LEFT; i <= BOTTOM; i=EDGE(i+1))
  {
    EdgePolyData[i]->GetLines()->Reset();
    EdgePolyData[i]->GetLines()->Allocate(EdgePolyData[i]->GetLines()->EstimateSize(1,2));
    EdgePolyData[i]->GetLines()->InsertNextCell(2);
    EdgePolyData[i]->GetLines()->InsertCellPoint(i);
    EdgePolyData[i]->GetLines()->InsertCellPoint((i+1)%4);
  }
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::UpdateRegion()
{
  switch (m_plane)
  {
    case Plane::XY:
      UpdateXYFace();
      break;
    case Plane::YZ:
      UpdateYZFace();
      break;
    case Plane::XZ:
      UpdateXZFace();
      break;
    default:
      Q_ASSERT(false);
      break;
  };
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::UpdateXYFace()
{
  auto depth = sliceDepth();

  double LB[3] = {LeftEdge,  BottomEdge, depth};
  double LT[3] = {LeftEdge,  TopEdge,    depth};
  double RT[3] = {RightEdge, TopEdge,    depth};
  double RB[3] = {RightEdge, BottomEdge, depth};

  Vertex->SetPoint(0, LB);
  Vertex->SetPoint(1, LT);
  Vertex->SetPoint(2, RT);
  Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    EdgePolyData[i]->Modified();
  }

  m_repBounds[0] = m_bounds[0] = std::min(LeftEdge, RightEdge );
  m_repBounds[1] = m_bounds[1] = std::max(LeftEdge, RightEdge );
  m_repBounds[2] = m_bounds[2] = std::min(TopEdge,  BottomEdge);
  m_repBounds[3] = m_bounds[3] = std::max(TopEdge,  BottomEdge);
  m_repBounds[4] = RB[2];
  m_repBounds[5] = RB[2];
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::UpdateYZFace()
{
  auto depth = sliceDepth();

  double LB[3] = {depth, BottomEdge, LeftEdge };
  double LT[3] = {depth, TopEdge,    LeftEdge };
  double RT[3] = {depth, TopEdge,    RightEdge};
  double RB[3] = {depth, BottomEdge, RightEdge};

  Vertex->SetPoint(0, LB);
  Vertex->SetPoint(1, LT);
  Vertex->SetPoint(2, RT);
  Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    EdgePolyData[i]->Modified();
  }

  m_repBounds[0] = LB[0];
  m_repBounds[1] = RB[0];
  m_repBounds[2] = m_bounds[2] = std::min(TopEdge,  BottomEdge);
  m_repBounds[3] = m_bounds[3] = std::max(TopEdge,  BottomEdge);
  m_repBounds[4] = m_bounds[4] = std::min(LeftEdge, RightEdge );
  m_repBounds[5] = m_bounds[5] = std::max(LeftEdge, RightEdge );
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::UpdateXZFace()
{
  auto depth = sliceDepth();

  double LB[3] = {LeftEdge,  depth, BottomEdge};
  double LT[3] = {LeftEdge,  depth, TopEdge};
  double RT[3] = {RightEdge, depth, TopEdge};
  double RB[3] = {RightEdge, depth, BottomEdge};

  Vertex->SetPoint(0, LB);
  Vertex->SetPoint(1, LT);
  Vertex->SetPoint(2, RT);
  Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    EdgePolyData[i]->Modified();
  }

  m_bounds[0] = std::min(LeftEdge, RightEdge );
  m_bounds[1] = std::max(LeftEdge, RightEdge );
  m_bounds[4] = std::min(TopEdge,  BottomEdge);
  m_bounds[5] = std::max(TopEdge,  BottomEdge);

  m_repBounds[0] = m_bounds[0];//LB[0];
  m_repBounds[1] = m_bounds[1];//RB[0];
  m_repBounds[2] = m_bounds[2];//LT[1];
  m_repBounds[3] = m_bounds[3];//LB[1];
  m_repBounds[4] = m_bounds[4];//RB[2];
  m_repBounds[5] = m_bounds[5];//RB[2];
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::SetDepth(double depth)
{
  m_depth = depth;

  UpdateRegion();
}
//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::SetPlane(Plane plane)
{
  if (plane == m_plane && plane != Plane::UNDEFINED) return;

  m_plane = plane;

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::SetSlice(double pos)
{
  m_slice = pos;

  int index = normalCoordinateIndex(m_plane);
  if (m_slice < m_bounds[2*index] || m_bounds[2*index+1] < m_slice)
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      EdgeActor[i]->SetProperty(InvisibleProperty);
    }
  }
  else
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    {
      EdgeActor[i]->SetProperty(EdgeProperty);
    }
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::SetOrthogonalBounds(double bounds[6])
{
  memcpy(m_bounds, bounds, 6*sizeof(double));

  LeftEdge   = m_repBounds[0] = m_bounds[2*hCoord()];
  RightEdge  = m_repBounds[1] = m_bounds[2*hCoord()+1];
  TopEdge    = m_repBounds[2] = m_bounds[2*vCoord()];
  BottomEdge = m_repBounds[3] = m_bounds[2*vCoord()+1];

  NumPoints = 4;
  NumSlices = 1;
  NumVertex = 4;

  SetSlice(m_slice);

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::GetOrthogonalBounds(double bounds[6])
{
  memcpy(bounds, m_bounds, 6*sizeof(double));
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::PlaceWidget(double bds[6])
{
//   std::cout << "Place Widget: ";
  int i;
  double bounds[6], center[3];

  AdjustBounds(bds,bounds,center);
//   std::cout << bds[0] << " "<< bds[1] << " "<< bds[2] << " "<< bds[3] << " "<< bds[4] << " "<< bds[5] << std::endl;
//   std::cout << bounds[0] << " "<< bounds[1] << " "<< bounds[2] << " "<< bounds[3] << " "<< bounds[4] << " "<< bounds[5] << std::endl;

  Vertex->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  Vertex->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  Vertex->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  Vertex->SetPoint(3, bounds[0], bounds[3], bounds[4]);

  for (i=0; i<6; i++)
  {
    InitialBounds[i] = bounds[i];
  }

  InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

  ValidPick = 1; //since we have set up widget
}

//----------------------------------------------------------------------------
int vtkOrthogonalRepresentation2D::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!Renderer || !Renderer->IsInViewport(X, Y))
  {
    InteractionState = vtkOrthogonalRepresentation2D::Outside;
    return InteractionState;
  }

  vtkAssemblyPath *path;
  // Try and pick a handle first
  LastPicker = nullptr;
  CurrentEdge = nullptr;
  EdgePicker->Pick(X,Y,0.0,Renderer);
  path = EdgePicker->GetPath();
  if ( path != nullptr )
  {
    LastPicker = EdgePicker;
    ValidPick = 1;

    CurrentEdge =
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (CurrentEdge == EdgeActor[LEFT])
    {
      InteractionState = vtkOrthogonalRepresentation2D::MoveLeft;
    }
    else if (CurrentEdge == EdgeActor[RIGHT])
    {
      InteractionState = vtkOrthogonalRepresentation2D::MoveRight;
    }
    else if (CurrentEdge == EdgeActor[TOP])
    {
      InteractionState = vtkOrthogonalRepresentation2D::MoveTop;
    }
    else if (CurrentEdge == EdgeActor[BOTTOM])
    {
      InteractionState = vtkOrthogonalRepresentation2D::MoveBottom;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    double pickPoint[3];
    vtkInteractorObserver::ComputeDisplayToWorld(Renderer, X, Y, 0, pickPoint);
    if ((LeftEdge < pickPoint[hCoord()] && pickPoint[hCoord()] < RightEdge)
     && (TopEdge  < pickPoint[vCoord()] && pickPoint[vCoord()] < BottomEdge))
    {
      InteractionState = vtkOrthogonalRepresentation2D::Inside;
    }
    else
    {
      InteractionState = vtkOrthogonalRepresentation2D::Outside;
    }
  }

  return InteractionState;
}

//----------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = state < vtkOrthogonalRepresentation2D::Outside ? vtkOrthogonalRepresentation2D::Outside : state;

  // Depending on state, highlight appropriate parts of representation
  InteractionState = state;
  switch (state)
    {
    case vtkOrthogonalRepresentation2D::MoveLeft:
    case vtkOrthogonalRepresentation2D::MoveRight:
    case vtkOrthogonalRepresentation2D::MoveTop:
    case vtkOrthogonalRepresentation2D::MoveBottom:
      HighlightEdge(CurrentEdge);
      break;
    case vtkOrthogonalRepresentation2D::Translating:
      Highlight();
      break;
    default:
      HighlightEdge(nullptr);
      break;
    }
}

//----------------------------------------------------------------------
double *vtkOrthogonalRepresentation2D::GetBounds()
{
  return m_repBounds;
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::BuildRepresentation()
{
  // Rebuild only if necessary
  if (GetMTime() > BuildTime ||
    (Renderer && Renderer->GetVTKWindow() &&
    (Renderer->GetVTKWindow()->GetMTime() > BuildTime ||
    Renderer->GetActiveCamera()->GetMTime() > BuildTime)) )
  {
    BuildTime.Modified();

    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::ReleaseGraphicsResources(vtkWindow *w)
{
  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    EdgeActor[i]->ReleaseGraphicsResources(w);
  }
}

//----------------------------------------------------------------------------
int vtkOrthogonalRepresentation2D::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    count += EdgeActor[i]->RenderOpaqueGeometry(v);
  }

  return count;
}

//----------------------------------------------------------------------------
int vtkOrthogonalRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    count += EdgeActor[i]->RenderTranslucentPolygonalGeometry(v);
  }

  return count;
}

//----------------------------------------------------------------------------
int vtkOrthogonalRepresentation2D::HasTranslucentPolygonalGeometry()
{
  int result=0;

  BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
  {
    result |= EdgeActor[i]->HasTranslucentPolygonalGeometry();
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::HighlightEdge(vtkSmartPointer<vtkActor> actor)
{
  for (EDGE edge=LEFT; edge <= BOTTOM; edge = EDGE(edge+1))
  {
    if (EdgeActor[edge] == actor)
    {
      EdgeActor[edge]->SetProperty(SelectedEdgeProperty);
    }
    else
    {
      EdgeActor[edge]->SetProperty(EdgeProperty);
    }
  }
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::Highlight()
{
  for (EDGE edge=LEFT; edge <= BOTTOM; edge = EDGE(edge+1))
  {
    EdgeActor[edge]->SetProperty(SelectedEdgeProperty);
  }
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  double *bounds=InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") "
     << "(" << bounds[4] << "," << bounds[5] << ")\n";
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::setRepresentationColor(double *color)
{
  if (0 != memcmp(m_color, color, sizeof(double)*3))
  {
    memcpy(m_color, color, sizeof(double)*3);

    updateEdgeColor(EdgeProperty);
    updateEdgeColor(SelectedEdgeProperty);
  }
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::setRepresentationPattern(int pattern)
{
  if (m_pattern != pattern)
  {
    m_pattern = pattern;

    updateEdgePattern(EdgeProperty);
    updateEdgePattern(SelectedEdgeProperty);
  }
}

//----------------------------------------------------------------------------
double vtkOrthogonalRepresentation2D::sliceDepth() const
{
  return m_slice + m_depth;
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::updateEdgeColor(vtkProperty *edge)
{
  edge->SetColor(m_color);
  edge->Modified();
}

//----------------------------------------------------------------------------
void vtkOrthogonalRepresentation2D::updateEdgePattern(vtkProperty *edge)
{
  edge->SetLineStipplePattern(m_pattern);
  edge->Modified();
}
