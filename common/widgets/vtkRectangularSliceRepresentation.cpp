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

vtkStandardNewMacro(vtkRectangularSliceRepresentation);

//----------------------------------------------------------------------------
vtkRectangularSliceRepresentation::vtkRectangularSliceRepresentation()
: Plane(AXIAL)
, Slice(0)
, Init(false)
, NumPoints(4)
, NumSlices(1)
{
  // The initial state
  this->InteractionState = vtkRectangularSliceRepresentation::Outside;

  memset(this->Bounds, 0, 6*sizeof(double));

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
    this->EdgeMapper[i]->SetInput(this->EdgePolyData[i]);
    this->EdgeActor[i]->SetMapper(this->EdgeMapper[i]);
    this->EdgeActor[i]->SetProperty(this->EdgeProperty);

    this->EdgePicker->AddPickList(this->EdgeActor[i]);
  }

  // Define the point coordinates
  double bounds[6] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  this->PlaceWidget(bounds);

  this->CurrentEdge = NULL;
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
    {
    return;
    }
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

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  bool crossRightEdge = leftEdge() + shift + MIN_SLICE_SPACING >= rightEdge();
  if (!crossRightEdge)
  {
    Bounds[2*hCoord()] += shift;
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  bool crossLeftEdge = leftEdge() + MIN_SLICE_SPACING >= rightEdge() + shift;
  if (!crossLeftEdge)
  {
    Bounds[2*hCoord()+1] += shift;
    UpdateRegion();
  }
}
//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  double crossBottomEdge = topEdge() + shift + MIN_SLICE_SPACING >= bottomEdge();
  if (!crossBottomEdge)
  {
    Bounds[2*vCoord()] += shift;
    UpdateRegion();
  }
}
//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  double crossTopEdge = topEdge() + MIN_SLICE_SPACING >= bottomEdge() + shift;
  if (!crossTopEdge)
  {
    Bounds[2*vCoord()+1] += shift;
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::CreateDefaultProperties()
{
  // Edge properties
  this->EdgeProperty = vtkProperty::New();
  this->EdgeProperty->SetRepresentationToSurface();
  this->EdgeProperty->SetOpacity(1.0);
  this->EdgeProperty->SetColor(1.0,1.0,0.0);
  this->EdgeProperty->SetLineWidth(1.0);

  // Selected Edge properties
  this->SelectedEdgeProperty = vtkProperty::New();
  this->SelectedEdgeProperty->SetRepresentationToSurface();
  this->SelectedEdgeProperty->SetOpacity(1.0);
  this->SelectedEdgeProperty->SetColor(1.0,1.0,0.0);
  this->SelectedEdgeProperty->SetLineWidth(2.0);

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
  switch (Plane)
  {
    case AXIAL:
      UpdateXYFace();
      break;
    case SAGITTAL:
      UpdateYZFace();
      break;
    case CORONAL:
      UpdateXZFace();
      break;
  };
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateXYFace()
{
  double LB[3] = {Bounds[0], Bounds[3], -0.1};
  double LT[3] = {Bounds[0], Bounds[2], -0.1};
  double RT[3] = {Bounds[1], Bounds[2], -0.1};
  double RB[3] = {Bounds[1], Bounds[3], -0.1};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  RepBounds[0] = LB[0];
  RepBounds[1] = RB[0];
  RepBounds[2] = LT[1];
  RepBounds[3] = LB[1];
  RepBounds[4] = RB[2];
  RepBounds[5] = RB[2];
}


//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateYZFace()
{
  double LB[3] = {0.1, Bounds[3], Bounds[4]};
  double LT[3] = {0.1, Bounds[2], Bounds[4]};
  double RT[3] = {0.1, Bounds[2], Bounds[5]};
  double RB[3] = {0.1, Bounds[3], Bounds[5]};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  RepBounds[0] = LB[0];
  RepBounds[1] = RB[0];
  RepBounds[2] = LT[1];
  RepBounds[3] = LB[1];
  RepBounds[4] = LT[2];
  RepBounds[5] = RT[2];
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::UpdateXZFace()
{
  double LB[3] = {Bounds[0], 0.1, Bounds[5]};
  double LT[3] = {Bounds[0], 0.1, Bounds[4]};
  double RT[3] = {Bounds[1], 0.1, Bounds[4]};
  double RB[3] = {Bounds[1], 0.1, Bounds[5]};

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  RepBounds[0] = Bounds[0];//LB[0];
  RepBounds[1] = Bounds[1];//RB[0];
  RepBounds[2] = Bounds[2];//LT[1];
  RepBounds[3] = Bounds[3];//LB[1];
  RepBounds[4] = Bounds[4];//RB[2];
  RepBounds[5] = Bounds[5];//RB[2];
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::SetPlane(PlaneType plane)
{
  if (Plane == plane && Init)
    return;

  Init = true;
  Plane = plane;

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::SetSlice(double pos)
{
  Slice = pos;
  if (Slice < Bounds[2*Plane] || Bounds[2*Plane+1] < Slice)
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
void vtkRectangularSliceRepresentation::SetBounds(double bounds[6])
{
  memcpy(Bounds, bounds, 6*sizeof(double));

  this->NumPoints = 4;
  this->NumSlices = 1;
  this->NumVertex = 4;

  SetSlice(Slice);

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularSliceRepresentation::GetBounds(double bounds[6])
{
  memcpy(bounds, Bounds, 6*sizeof(double));
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
    {
    this->InitialBounds[i] = bounds[i];
    }
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
  this->LastPicker = NULL;
  this->CurrentEdge = NULL;
  this->EdgePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->EdgePicker->GetPath();
  if ( path != NULL )
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
      this->HighlightEdge(this->CurrentEdge);
      break;
    default:
      this->HighlightEdge(NULL);
    }
}

//----------------------------------------------------------------------
double *vtkRectangularSliceRepresentation::GetBounds()
{
  return RepBounds;
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
void vtkRectangularSliceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";
}