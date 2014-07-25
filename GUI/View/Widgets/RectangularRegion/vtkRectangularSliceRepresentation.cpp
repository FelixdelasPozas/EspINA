#include "vtkRectangularSliceRepresentation.h"

#include <vtkActor.h>
#include <vtkAssemblyPath.h>
#include <vtkBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkDoubleArray.h>
#include <vtkInteractorObserver.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkWindow.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

const double MIN_SLICE_SPACING = 2;

using namespace ESPINA;

vtkStandardNewMacro(vtkRectangularSliceRepresentation);

//----------------------------------------------------------------------------
vtkRectangularSliceRepresentation::vtkRectangularSliceRepresentation()
: Vertex              {nullptr}
, EdgePicker          {nullptr}
, LastPicker          {nullptr}
, CurrentEdge         {nullptr}
, EdgeProperty        {nullptr}
, SelectedEdgeProperty{nullptr}
, InvisibleProperty   {nullptr}
, m_plane             {Plane::UNDEFINED}
, Slice               {0}
, NumPoints           {4}
, NumSlices           {1}
, NumVertex           {0}
, LeftEdge            {0}
, TopEdge             {0}
, RightEdge           {1}
, BottomEdge          {1}
, m_pattern           {0xFFFF}
{
  // The initial state
  this->InteractionState = vtkRectangularSliceRepresentation::Outside;

  memset(m_bounds, 0, sizeof(double)*6);
  memset(m_repBounds, 0, sizeof(double)*6);

  // default representation color
  m_color[0] = m_color[1] = 1.0;
  m_color[2] = 0.0;

  this->CreateDefaultProperties();

  //Manage the picking stuff
  this->EdgePicker = vtkCellPicker::New();
  this->EdgePicker->SetTolerance(0.01);
  this->EdgePicker->PickFromListOn();

  // Build edges
  this->Vertex = vtkPoints::New(VTK_DOUBLE);
  this->Vertex->SetNumberOfPoints(4);//line sides;
  for (EDGE i=LEFT; i<=BOTTOM; i = EDGE(i+1))
  {
    this->EdgePolyData[i] = vtkPolyData::New();
    this->EdgeMapper[i]   = vtkPolyDataMapper::New();
    this->EdgeActor[i]    = vtkActor::New();

    this->EdgePolyData[i]->SetPoints(this->Vertex);
    this->EdgePolyData[i]->SetLines(vtkCellArray::New());
    this->EdgeMapper[i]->SetInputData(this->EdgePolyData[i]);
    this->EdgeActor[i]->SetMapper(this->EdgeMapper[i]);
    this->EdgeActor[i]->SetProperty(this->EdgeProperty);

    this->EdgePicker->AddPickList(this->EdgeActor[i]);
  }

  // Define the point coordinates
  double bounds[6] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  this->PlaceWidget(bounds);

  this->CurrentEdge = nullptr;
}

//----------------------------------------------------------------------------
vtkRectangularSliceRepresentation::~vtkRectangularSliceRepresentation()
{
  for(int i=0; i<4; i++)
  {
    this->EdgeActor[i]->Delete();
    this->EdgeMapper[i]->Delete();
    this->EdgePolyData[i]->Delete();
  }

  this->EdgePicker->Delete();

  this->EdgeProperty->Delete();
  this->SelectedEdgeProperty->Delete();
  this->InvisibleProperty->Delete();
}

//----------------------------------------------------------------------
void vtkRectangularSliceRepresentation::GetPolyData(vtkPolyData *pd)
{
//   pd->SetPoints(this->RegionPolyData->GetPoints());
//   pd->SetPolys(this->RegionPolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkRectangularSliceRepresentation::reset()
{
//   std::cout << "Shift's been reset" << std::endl;
  CreateRegion();
}


//----------------------------------------------------------------------
void vtkRectangularSliceRepresentation::StartWidgetInteraction(double e[2])
{
  // Store the start position
  this->StartEventPosition[0] = e[0];
  this->StartEventPosition[1] = e[1];
  this->StartEventPosition[2] = 0.0;

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;

  this->ComputeInteractionState(static_cast<int>(e[0]),static_cast<int>(e[1]),0);
}

//----------------------------------------------------------------------
void vtkRectangularSliceRepresentation::WidgetInteraction(double e[2])
{
  // Convert events to appropriate coordinate systems
  vtkCamera *camera = this->Renderer->GetActiveCamera();
  if ( !camera )
    return;

  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z, vpn[3];
  camera->GetViewPlaneNormal(vpn);

  // Compute the two points defining the motion vector
  double pos[3];
  if ( this->LastPicker == this->EdgePicker )
  {
    this->EdgePicker->GetPickPosition(pos);
  }
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( this->InteractionState == vtkRectangularSliceRepresentation::MoveLeft )
  {
    this->MoveLeftEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularSliceRepresentation::MoveRight )
  {
    this->MoveRightEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularSliceRepresentation::MoveTop )
  {
    this->MoveTopEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularSliceRepresentation::MoveBottom )
  {
    this->MoveBottomEdge(prevPickPoint,pickPoint);
  }
  else if ( this->InteractionState == vtkRectangularSliceRepresentation::Translating )
  {
    this->Translate(prevPickPoint,pickPoint);
  }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  LeftEdge     += shift;
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  RightEdge   += shift;
  UpdateRegion();
}
//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  TopEdge     += shift;
  UpdateRegion();
}
//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  BottomEdge  += shift;
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::Translate(double* p1, double* p2)
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
void vtkRectangularSliceRepresentation::CreateDefaultProperties()
{
  // Edge properties
  this->EdgeProperty = vtkProperty::New();
  this->EdgeProperty->SetRepresentationToSurface();
  this->EdgeProperty->SetOpacity(1.0);
  this->EdgeProperty->SetColor(m_color);
  this->EdgeProperty->SetLineWidth(1.0);
  this->EdgeProperty->SetLineStipplePattern(m_pattern);

  // Selected Edge properties
  this->SelectedEdgeProperty = vtkProperty::New();
  this->SelectedEdgeProperty->SetRepresentationToSurface();
  this->SelectedEdgeProperty->SetOpacity(1.0);
  this->SelectedEdgeProperty->SetColor(m_color);
  this->SelectedEdgeProperty->SetLineWidth(2.0);
  this->SelectedEdgeProperty->SetLineStipplePattern(m_pattern);

  this->InvisibleProperty = vtkProperty::New();
  this->InvisibleProperty->SetRepresentationToWireframe();
  this->InvisibleProperty->SetAmbient(0.0);
  this->InvisibleProperty->SetDiffuse(0.0);
  this->InvisibleProperty->SetOpacity(0);
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::CreateRegion()
{
  // Corners of the rectangular region
  this->Vertex->SetNumberOfPoints(4);

  for(EDGE i=LEFT; i <= BOTTOM; i=EDGE(i+1))
  {
    this->EdgePolyData[i]->GetLines()->Reset();
    this->EdgePolyData[i]->GetLines()->Allocate(
      this->EdgePolyData[i]->GetLines()->EstimateSize(1,2));
    this->EdgePolyData[i]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint(i);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint((i+1)%4);
  }
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateRegion()
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
void vtkRectangularSliceRepresentation::UpdateXYFace()
{
  double LB[3] = {LeftEdge,  BottomEdge, -0.1};
  double LT[3] = {LeftEdge,  TopEdge,    -0.1};
  double RT[3] = {RightEdge, TopEdge,    -0.1};
  double RB[3] = {RightEdge, BottomEdge, -0.1};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  m_repBounds[0] = m_bounds[0] = std::min(LeftEdge, RightEdge );
  m_repBounds[1] = m_bounds[1] = std::max(LeftEdge, RightEdge );
  m_repBounds[2] = m_bounds[2] = std::min(TopEdge,  BottomEdge);
  m_repBounds[3] = m_bounds[3] = std::max(TopEdge,  BottomEdge);
  m_repBounds[4] = RB[2];
  m_repBounds[5] = RB[2];
}


//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateYZFace()
{
  double LB[3] = {0.1, BottomEdge, LeftEdge };
  double LT[3] = {0.1, TopEdge,    LeftEdge };
  double RT[3] = {0.1, TopEdge,    RightEdge};
  double RB[3] = {0.1, BottomEdge, RightEdge};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  m_repBounds[0] = LB[0];
  m_repBounds[1] = RB[0];
  m_repBounds[2] = m_bounds[2] = std::min(TopEdge,  BottomEdge);
  m_repBounds[3] = m_bounds[3] = std::max(TopEdge,  BottomEdge);
  m_repBounds[4] = m_bounds[4] = std::min(LeftEdge, RightEdge );
  m_repBounds[5] = m_bounds[5] = std::max(LeftEdge, RightEdge );
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateXZFace()
{
  double LB[3] = {LeftEdge, 0.1, BottomEdge};
  double LT[3] = {LeftEdge, 0.1, TopEdge};
  double RT[3] = {RightEdge, 0.1, TopEdge};
  double RB[3] = {RightEdge, 0.1, BottomEdge};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

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
void vtkRectangularSliceRepresentation::SetPlane(Plane plane)
{
  if (plane == m_plane && plane != Plane::UNDEFINED)
    return;

  m_plane = plane;

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::SetSlice(double pos)
{
  Slice = pos;

  int index = normalCoordinateIndex(m_plane);
  if (Slice < m_bounds[2*index] || m_bounds[2*index+1] < Slice)
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(InvisibleProperty);
    return;
  } else
  {
    for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
      this->EdgeActor[i]->SetProperty(EdgeProperty);
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::SetCuboidBounds(double bounds[6])
{
  memcpy(m_bounds, bounds, 6*sizeof(double));

  LeftEdge   = m_repBounds[0] = m_bounds[2*hCoord()];
  RightEdge  = m_repBounds[1] = m_bounds[2*hCoord()+1];
  TopEdge    = m_repBounds[2] = m_bounds[2*vCoord()];
  BottomEdge = m_repBounds[3] = m_bounds[2*vCoord()+1];

  this->NumPoints = 4;
  this->NumSlices = 1;
  this->NumVertex = 4;

  SetSlice(Slice);

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::GetCuboidBounds(double bounds[6])
{
  memcpy(bounds, m_bounds, 6*sizeof(double));
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::PlaceWidget(double bds[6])
{
//   std::cout << "Place Widget: ";
  int i;
  double bounds[6], center[3];

  this->AdjustBounds(bds,bounds,center);
//   std::cout << bds[0] << " "<< bds[1] << " "<< bds[2] << " "<< bds[3] << " "<< bds[4] << " "<< bds[5] << std::endl;
//   std::cout << bounds[0] << " "<< bounds[1] << " "<< bounds[2] << " "<< bounds[3] << " "<< bounds[4] << " "<< bounds[5] << std::endl;

  this->Vertex->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  this->Vertex->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  this->Vertex->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  this->Vertex->SetPoint(3, bounds[0], bounds[3], bounds[4]);

  for (i=0; i<6; i++)
    this->InitialBounds[i] = bounds[i];

  this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

  this->ValidPick = 1; //since we have set up widget
}

//----------------------------------------------------------------------------
int vtkRectangularSliceRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
  {
    this->InteractionState = vtkRectangularSliceRepresentation::Outside;
    return this->InteractionState;
  }

  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = nullptr;
  this->CurrentEdge = nullptr;
  this->EdgePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->EdgePicker->GetPath();
  if ( path != nullptr )
  {
    this->LastPicker = this->EdgePicker;
    this->ValidPick = 1;

    this->CurrentEdge =
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (this->CurrentEdge == this->EdgeActor[LEFT])
    {
      this->InteractionState = vtkRectangularSliceRepresentation::MoveLeft;
    }
    else if (this->CurrentEdge == this->EdgeActor[RIGHT])
    {
      this->InteractionState = vtkRectangularSliceRepresentation::MoveRight;
    } 
    else if (this->CurrentEdge == this->EdgeActor[TOP])
    {
      this->InteractionState = vtkRectangularSliceRepresentation::MoveTop;
    } 
    else if (this->CurrentEdge == this->EdgeActor[BOTTOM])
    {
      this->InteractionState = vtkRectangularSliceRepresentation::MoveBottom;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    double pickPoint[3];
    vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, X, Y, 0, pickPoint);
    if ((LeftEdge < pickPoint[hCoord()] && pickPoint[hCoord()] < RightEdge)
     && (TopEdge  < pickPoint[vCoord()] && pickPoint[vCoord()] < BottomEdge))
      this->InteractionState = vtkRectangularSliceRepresentation::Inside;
    else
      this->InteractionState = vtkRectangularSliceRepresentation::Outside;
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkRectangularSliceRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = state < vtkRectangularSliceRepresentation::Outside ? vtkRectangularSliceRepresentation::Outside : state;

  // Depending on state, highlight appropriate parts of representation
  this->InteractionState = state;
  switch (state)
    {
    case vtkRectangularSliceRepresentation::MoveLeft:
    case vtkRectangularSliceRepresentation::MoveRight:
    case vtkRectangularSliceRepresentation::MoveTop:
    case vtkRectangularSliceRepresentation::MoveBottom:
      this->HighlightEdge(this->CurrentEdge);
      break;
    case vtkRectangularSliceRepresentation::Translating:
      this->Highlight();
      break;
    default:
      this->HighlightEdge(nullptr);
      break;
    }
}

//----------------------------------------------------------------------
double *vtkRectangularSliceRepresentation::GetBounds()
{
  return m_repBounds;
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::BuildRepresentation()
{
  // Rebuild only if necessary
  if ( this->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        (this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime ||
        this->Renderer->GetActiveCamera()->GetMTime() > this->BuildTime)) )
  {
    this->BuildTime.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgeActor[i]->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkRectangularSliceRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularSliceRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularSliceRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    result |= this->EdgeActor[i]->HasTranslucentPolygonalGeometry();

  return result;
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::HighlightEdge(vtkActor* actor)
{
  for (EDGE edge=LEFT; edge <= BOTTOM; edge = EDGE(edge+1))
  {
    if (this->EdgeActor[edge] == actor)
      this->EdgeActor[edge]->SetProperty(this->SelectedEdgeProperty);
    else
      this->EdgeActor[edge]->SetProperty(this->EdgeProperty);
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::Highlight()
{
  for (EDGE edge=LEFT; edge <= BOTTOM; edge = EDGE(edge+1))
    this->EdgeActor[edge]->SetProperty(this->SelectedEdgeProperty);
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::setRepresentationColor(double *color)
{
  if (0 == memcmp(m_color, color, sizeof(double)*3))
    return;

  memcpy(m_color, color, sizeof(double)*3);
  this->EdgeProperty->SetColor(m_color);
  this->EdgeProperty->Modified();
  this->SelectedEdgeProperty->SetColor(m_color);
  this->SelectedEdgeProperty->Modified();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::setRepresentationPattern(int pattern)
{
  if (m_pattern == pattern)
    return;

  m_pattern = pattern;
  this->EdgeProperty->SetLineStipplePattern(m_pattern);
  this->EdgeProperty->Modified();
  this->SelectedEdgeProperty->SetLineStipplePattern(m_pattern);
  this->SelectedEdgeProperty->Modified();
}
