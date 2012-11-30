#include "vtkBoundingRegionSliceRepresentation.h"

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


//----------------------------------------------------------------------------
vtkBoundingRegionSliceRepresentation::vtkBoundingRegionSliceRepresentation()
: Region(NULL)
, Slice(0)
, Init(false)
, NumPoints(0)
, NumSlices(0)
{
  // The initial state
  this->InteractionState = vtkBoundingRegionSliceRepresentation::Outside;

  Resolution[0] = Resolution[1] = Resolution[2] = 1;

  memset(this->InclusionOffset, 0, 3*sizeof(double));
  memset(this->ExclusionOffset, 0, 3*sizeof(double));

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
    if (i < RIGHT)
      this->EdgeActor[i]->SetProperty(this->InclusionEdgeProperty);
    else
      this->EdgeActor[i]->SetProperty(this->ExclusionEdgeProperty);

    this->EdgePicker->AddPickList(this->EdgeActor[i]);
  }

  // Define the point coordinates
  double bounds[6] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  this->PlaceWidget(bounds);

  this->CurrentEdge = NULL;
}

//----------------------------------------------------------------------------
vtkBoundingRegionSliceRepresentation::~vtkBoundingRegionSliceRepresentation()
{
  for(int i=0; i<4; i++)
  {
    this->EdgeActor[i]->Delete();
    this->EdgeMapper[i]->Delete();
    this->EdgePolyData[i]->Delete();
  }

  this->EdgePicker->Delete();

  this->InclusionEdgeProperty->Delete();
  this->ExclusionEdgeProperty->Delete();
  this->SelectedInclusionProperty->Delete();
  this->SelectedExclusionProperty->Delete();
  this->InvisibleProperty->Delete();
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::reset()
{
//   std::cout << "Shift's been reset" << std::endl;
  memset(this->InclusionOffset, 0, 3*sizeof(Nm));
  memset(this->ExclusionOffset, 0, 3*sizeof(Nm));
  CreateRegion();
}


//----------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::StartWidgetInteraction(double e[2])
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
void vtkBoundingRegionSliceRepresentation::WidgetInteraction(double e[2])
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
  if ( this->InteractionState == vtkBoundingRegionSliceRepresentation::MoveLeft )
  {
    this->MoveLeftEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkBoundingRegionSliceRepresentation::MoveRight )
  {
    this->MoveRightEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkBoundingRegionSliceRepresentation::MoveTop )
  {
    this->MoveTopEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkBoundingRegionSliceRepresentation::MoveBottom )
  {
    this->MoveBottomEdge(prevPickPoint,pickPoint);
  }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::CreateDefaultProperties()
{
  // Edge properties
  this->InclusionEdgeProperty = vtkProperty::New();
  this->InclusionEdgeProperty->SetRepresentationToSurface();
  this->InclusionEdgeProperty->SetOpacity(1.0);
  this->InclusionEdgeProperty->SetColor(0.0,1.0,0.0);
  this->InclusionEdgeProperty->SetLineWidth(1.0);

  this->ExclusionEdgeProperty = vtkProperty::New();
  this->ExclusionEdgeProperty->SetRepresentationToSurface();
  this->ExclusionEdgeProperty->SetOpacity(1.0);
  this->ExclusionEdgeProperty->SetColor(1.0,0.0,0.0);
  this->ExclusionEdgeProperty->SetLineWidth(1.0);

  // Selected Edge properties
  this->SelectedInclusionProperty = vtkProperty::New();
  this->SelectedInclusionProperty->SetRepresentationToSurface();
  this->SelectedInclusionProperty->SetOpacity(1.0);
  this->SelectedInclusionProperty->SetColor(0.0,1.0,0.0);
  this->SelectedInclusionProperty->SetLineWidth(2.0);
  this->SelectedExclusionProperty = vtkProperty::New();
  this->SelectedExclusionProperty->SetRepresentationToSurface();
  this->SelectedExclusionProperty->SetOpacity(1.0);
  this->SelectedExclusionProperty->SetColor(1.0,0.0,0.0);
  this->SelectedExclusionProperty->SetLineWidth(2.0);

  this->InvisibleProperty = vtkProperty::New();
  this->InvisibleProperty->SetRepresentationToWireframe();
  this->InvisibleProperty->SetAmbient(0.0);
  this->InvisibleProperty->SetDiffuse(0.0);
  this->InvisibleProperty->SetOpacity(0);
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::regionBounds(int regionSlice,
                                                        Nm bounds[6])
{
  if (regionSlice < 0)
    vtkMath::UninitializeBounds(bounds);
  else
  {
    double p1[3], p2[3];
    Region->GetPoint(regionSlice*4+0, p1);
    Region->GetPoint(regionSlice*4+2, p2);

    bounds[0] = p1[0];
    bounds[1] = p2[0];
    bounds[2] = p2[1];
    bounds[3] = p1[1];
    bounds[4] = bounds[5] = p1[2];
  }
}

//----------------------------------------------------------------------------
int vtkBoundingRegionSliceRepresentation::sliceNumber(Nm pos) const
{
  double point[3];
  for (int number = 0; number < NumSlices; number++)
  {
    this->Region->GetPoints()->GetPoint(4*number, point);
//     this->Region->GetOutput()->GetPoints()->GetPoint(4*(number+1), next);
//     if (point[Plane] <= pos && pos < next[Plane])
    if (pos <= point[2])
      return number;
  }
  return NumSlices-1;
}

// double slope(double p1[2], double p2[2])
// {
//   return (p2[1] - p1[1])/(p2[0] - p1[0]);
// }
// double interpolate(double p1[2], double p2[2], double x)
// {
//   double m = slope(p1, p2);
//   return m*x + p1[1] - m*p1[0];
// }
// 
// void intersection(double A1[2], double A2[2], double B1[2], double B2[2], double p[2])
// {
//   double mA = slope(A1, A2);
//   double bA = A1[1] / (A1[0]*mA);
//   double mB = slope(B1, B2);
//   double bB = B1[1] / (B1[0]*mB);
//   p[0] = (bB - bA)/(mA - mB);
//   p[1] = mA*A1[0] + bA;
// }

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::SetSlice(Nm pos)
{
  Slice = pos;
//   std::cout << "Plane: " << Plane << ", Slice: " << pos << /*", Spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2] <<*/ std::endl;
//   if (pos < InclusionOffset[Plane])// || NumSlices <= Slice)
//   {
//     for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
//       this->EdgeActor[i]->SetProperty(InvisibleProperty);
//     return;
//   } else
//   {
//     for(EDGE i = LEFT; i <= TOP; i = EDGE(i+1))
//       this->EdgeActor[i]->SetProperty(InclusionEdgeProperty);
//     for(EDGE i = RIGHT; i <= BOTTOM; i = EDGE(i+1))
//       this->EdgeActor[i]->SetProperty(ExclusionEdgeProperty);
//   }
  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::SetBoundingRegion(vtkSmartPointer<vtkPolyData> region,
                                                             Nm inclusionOffset[3],
                                                             Nm exclusionOffset[3],
                                                             Nm slicingStep[3])
{
  Region = region;
  memcpy(InclusionOffset, inclusionOffset, 3*sizeof(Nm));
  memcpy(ExclusionOffset, exclusionOffset, 3*sizeof(Nm));
  memcpy(Resolution, slicingStep, 3*sizeof(Nm));

  this->Region->Update();
  this->NumPoints = this->Region->GetPoints()->GetNumberOfPoints();
  this->NumSlices = this->NumPoints / 4;
  this->NumVertex = this->NumSlices * 2;

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::PlaceWidget(double bds[6])
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
int vtkBoundingRegionSliceRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkBoundingRegionSliceRepresentation::Outside;
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
      this->InteractionState = vtkBoundingRegionSliceRepresentation::MoveLeft;
    }
    else if (this->CurrentEdge == this->EdgeActor[RIGHT])
    {
      this->InteractionState = vtkBoundingRegionSliceRepresentation::MoveRight;
    } 
    else if (this->CurrentEdge == this->EdgeActor[TOP])
    {
      this->InteractionState = vtkBoundingRegionSliceRepresentation::MoveTop;
    } 
    else if (this->CurrentEdge == this->EdgeActor[BOTTOM])
    {
      this->InteractionState = vtkBoundingRegionSliceRepresentation::MoveBottom;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    this->InteractionState = vtkBoundingRegionSliceRepresentation::Outside;
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = state < vtkBoundingRegionSliceRepresentation::Outside ? vtkBoundingRegionSliceRepresentation::Outside : state;

  // Depending on state, highlight appropriate parts of representation
  this->InteractionState = state;
  switch (state)
    {
    case vtkBoundingRegionSliceRepresentation::MoveLeft:
    case vtkBoundingRegionSliceRepresentation::MoveRight:
    case vtkBoundingRegionSliceRepresentation::MoveTop:
    case vtkBoundingRegionSliceRepresentation::MoveBottom:
      this->HighlightEdge(this->CurrentEdge);
      break;
    case vtkBoundingRegionSliceRepresentation::Translating:
      this->HighlightEdge(this->CurrentEdge);
      break;
    default:
      this->HighlightEdge(NULL);
    }
}

//----------------------------------------------------------------------
double *vtkBoundingRegionSliceRepresentation::GetBounds()
{
  this->BuildRepresentation();
  return Region->GetBounds();
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::BuildRepresentation()
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
void vtkBoundingRegionSliceRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgeActor[i]->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkBoundingRegionSliceRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkBoundingRegionSliceRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkBoundingRegionSliceRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    result |= this->EdgeActor[i]->HasTranslucentPolygonalGeometry();

  return result;
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::HighlightEdge(vtkActor* actor)
{
  for (EDGE edge=LEFT; edge <= BOTTOM; edge = EDGE(edge+1))
  {
    if (this->EdgeActor[edge] == actor)
    {
      if (edge < RIGHT)
	this->EdgeActor[edge]->SetProperty(this->SelectedInclusionProperty);
      else
	this->EdgeActor[edge]->SetProperty(this->SelectedExclusionProperty);
    }
    else if (edge < RIGHT)
      this->EdgeActor[edge]->SetProperty(this->InclusionEdgeProperty);
    else
      this->EdgeActor[edge]->SetProperty(this->ExclusionEdgeProperty);
  }
}

//----------------------------------------------------------------------------
void vtkBoundingRegionSliceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";
}