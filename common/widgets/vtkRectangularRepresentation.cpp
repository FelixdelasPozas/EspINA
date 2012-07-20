#include "vtkRectangularRepresentation.h"

#include <vtkActor.h>
#include <vtkAssemblyPath.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkDoubleArray.h>
#include <vtkInteractorObserver.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkWindow.h>

vtkStandardNewMacro(vtkRectangularRepresentation);

//----------------------------------------------------------------------------
vtkRectangularRepresentation::vtkRectangularRepresentation()
: Plane(AXIAL)
{
  // The initial state
  this->InteractionState = vtkRectangularRepresentation::Outside;

  this->CreateDefaultProperties();

  this->EdgePicker = vtkCellPicker::New();
  this->EdgePicker->SetTolerance(0.01);
  this->EdgePicker->PickFromListOn();
  this->RegionPicker = vtkCellPicker::New();
  this->RegionPicker->SetTolerance(0.01);
  this->RegionPicker->PickFromListOn();

  // Rectangle's corners
  this->Vertex = vtkPoints::New(VTK_DOUBLE);
  this->Vertex->SetNumberOfPoints(4);

  // Build edges
  for (unsigned int i=0; i<4; i++)
  {
    this->EdgePolyData[i] = vtkPolyData::New();
    this->EdgeMapper[i]   = vtkPolyDataMapper::New();
    this->EdgeActor[i]    = vtkActor::New();

    this->EdgePolyData[i]->SetPoints(this->Vertex);
    this->EdgePolyData[i]->SetLines(vtkCellArray::New());
    this->EdgePolyData[i]->GetLines()->Allocate(
      this->EdgePolyData[i]->GetLines()->EstimateSize(1,2));
    this->EdgePolyData[i]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint(i);
    this->EdgePolyData[i]->GetLines()->InsertCellPoint((i+1)%4);

    this->EdgeMapper[i]->SetInput(this->EdgePolyData[i]);

    this->EdgeActor[i]->SetMapper(this->EdgeMapper[i]);
    this->EdgeActor[i]->SetProperty(this->EdgeProperty);

    this->EdgePicker->AddPickList(this->EdgeActor[i]);
  }

  // Build contained region
  this->RegionPolyData = vtkPolyData::New();
  this->RegionMapper   = vtkPolyDataMapper::New();
  this->RegionActor    = vtkActor::New();

  this->RegionPolyData->SetPoints(this->Vertex);
  vtkCellArray *region = vtkCellArray::New();
  region->Allocate(region->EstimateSize(1,4));
  vtkIdType vertex[4] = {0, 1, 2, 3};
  region->InsertNextCell(4, vertex);
  this->RegionPolyData->SetPolys(region);
  this->RegionPolyData->Modified();
  this->RegionMapper->SetInput(this->RegionPolyData);
  this->RegionActor->SetMapper(this->RegionMapper);
  this->RegionActor->SetProperty(this->RegionProperty);

  this->RegionPicker->AddPickList(this->RegionActor);

  // Define the point coordinates
  double bounds[6] = {-1.5, 1.5, -0.5, 0.5, -0.5, 0.5};
  this->BoundingBox = vtkBox::New();
  this->PlaceWidget(bounds);

  this->CurrentHandle = NULL;

  BuildRepresentation();
}

//----------------------------------------------------------------------------
vtkRectangularRepresentation::~vtkRectangularRepresentation()
{
  for(int i=0; i<4; i++)
  {
    this->EdgeActor[i]->Delete();
    this->EdgeMapper[i]->Delete();
    this->EdgePolyData[i]->Delete();
  }

  this->RegionActor->Delete();
  this->RegionMapper->Delete();
  this->RegionPolyData->Delete();

  this->EdgePicker->Delete();
  this->RegionPicker->Delete();

  this->BoundingBox->Delete();

  this->EdgeProperty->Delete();
  this->RegionProperty->Delete();
  this->SelectedEdgeProperty->Delete();
  this->InvisibleProperty->Delete();
}

//----------------------------------------------------------------------
void vtkRectangularRepresentation::reset()
{
//   std::cout << "Reset Widget" << std::endl;
  updateVertex();
}

//----------------------------------------------------------------------
void vtkRectangularRepresentation::StartWidgetInteraction(double e[2])
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
void vtkRectangularRepresentation::WidgetInteraction(double e[2])
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
  } else if (this->LastPicker == this->RegionPicker)
  {
    this->RegionPicker->GetPickPosition(pos);
  }
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( this->InteractionState == vtkRectangularRepresentation::MoveLeft )
    {
    this->MoveLeftEdge(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularRepresentation::MoveRight )
    {
    this->MoveRightEdge(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularRepresentation::MoveTop )
    {
    this->MoveTopEdge(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularRepresentation::MoveBottom )
    {
    this->MoveBottomEdge(prevPickPoint,pickPoint);
    }
  else if ( this->InteractionState == vtkRectangularRepresentation::Translating )
    {
    this->Translate(prevPickPoint, pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularRepresentation::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint, 
                static_cast<int>(e[0]), static_cast<int>(e[1]));
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}


//----------------------------------------------------------------------------
void vtkRectangularRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  this->InitialBounds[leftIndex()] = p2[hCoord()];
  updateVertex();
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::MoveRightEdge(double* p1, double* p2)
{
  this->InitialBounds[rightIndex()] = p2[hCoord()];
  updateVertex();
}
//----------------------------------------------------------------------------
void vtkRectangularRepresentation::MoveTopEdge(double* p1, double* p2)
{
  this->InitialBounds[topIndex()] = p2[vCoord()];
  updateVertex();
}
//----------------------------------------------------------------------------
void vtkRectangularRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  this->InitialBounds[bottomIndex()] = p2[vCoord()];
  updateVertex();
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::Translate(double *p1, double *p2)
{
  double v[3];

  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  this->InitialBounds[leftIndex()]   += v[hCoord()];
  this->InitialBounds[rightIndex()]  += v[hCoord()];
  this->InitialBounds[topIndex()]    += v[vCoord()];
  this->InitialBounds[bottomIndex()] += v[vCoord()];
  updateVertex();
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::Scale(double *vtkNotUsed(p1),
                                 double *vtkNotUsed(p2),
                                 int vtkNotUsed(X),
                                 int Y)
{
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::CreateDefaultProperties()
{
  this->EdgeProperty = vtkProperty::New();
  this->EdgeProperty->SetRepresentationToSurface();
  this->EdgeProperty->SetLineWidth(2.0);
  this->EdgeProperty->SetColor(1,1,0);

  this->RegionProperty = vtkProperty::New();
  this->RegionProperty->SetRepresentationToSurface();
  this->RegionProperty->SetLineWidth(2.0);
  this->RegionProperty->SetColor(1,1,0);
  this->RegionProperty->SetOpacity(0.1);

  this->SelectedEdgeProperty = vtkProperty::New();
  this->SelectedEdgeProperty->SetRepresentationToWireframe();
  this->SelectedEdgeProperty->SetAmbient(1.0);
  this->SelectedEdgeProperty->SetAmbientColor(0.0,1.0,0.0);
  this->SelectedEdgeProperty->SetLineWidth(2.0);

  this->InvisibleProperty = vtkProperty::New();
  this->InvisibleProperty->SetRepresentationToWireframe();
  this->InvisibleProperty->SetAmbient(0.0);
  this->InvisibleProperty->SetDiffuse(0.0);
  this->InvisibleProperty->SetOpacity(0);
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::updateVertex()
{
  switch (Plane)
  {
    case AXIAL:
      this->Vertex->SetPoint(0, InitialBounds[0], InitialBounds[2], -0.1);
      this->Vertex->SetPoint(1, InitialBounds[1], InitialBounds[2], -0.1);
      this->Vertex->SetPoint(2, InitialBounds[1], InitialBounds[3], -0.1);
      this->Vertex->SetPoint(3, InitialBounds[0], InitialBounds[3], -0.1);
      break;
    case CORONAL:
      this->Vertex->SetPoint(0, InitialBounds[0], 0.1, InitialBounds[4]);
      this->Vertex->SetPoint(1, InitialBounds[1], 0.1, InitialBounds[4]);
      this->Vertex->SetPoint(2, InitialBounds[1], 0.1, InitialBounds[5]);
      this->Vertex->SetPoint(3, InitialBounds[0], 0.1, InitialBounds[5]);
      break;
    case SAGITTAL:
      this->Vertex->SetPoint(0, 0.1, InitialBounds[2], InitialBounds[4]);
      this->Vertex->SetPoint(1, 0.1, InitialBounds[2], InitialBounds[5]);
      this->Vertex->SetPoint(2, 0.1, InitialBounds[3], InitialBounds[5]);
      this->Vertex->SetPoint(3, 0.1, InitialBounds[3], InitialBounds[4]);
      break;
  }

  for(int i=0; i<4; i++)
    this->EdgePolyData[i]->Modified();
  this->RegionPolyData->Modified();
}


//----------------------------------------------------------------------------
void vtkRectangularRepresentation::SetPlane(PlaneType plane)
{
  if (Plane == plane)
    return;

  Plane = plane;
  updateVertex();
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::PlaceWidget(double bds[6])
{
  int i;
  double bounds[6], center[3];

//   this->AdjustBounds(bds,bounds,center);
//   std::cout << bds[0] << " "<< bds[1] << " "<< bds[2] << " "<< bds[3] << " "<< bds[4] << " "<< bds[5] << std::endl;
//   std::cout << bounds[0] << " "<< bounds[1] << " "<< bounds[2] << " "<< bounds[3] << " "<< bounds[4] << " "<< bounds[5] << std::endl;
  memcpy(bounds, bds, 6*sizeof(double));

  for (i=0; i<6; i++)
    {
    this->InitialBounds[i] = bounds[i];
    }
  this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

  this->ValidPick = 1; //since we have set up widget
  updateVertex();
}

//----------------------------------------------------------------------------
int vtkRectangularRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkRectangularRepresentation::Outside;
    return this->InteractionState;
    }
  
  vtkAssemblyPath *path;
  // Try and pick an edge first
  this->LastPicker = NULL;
  this->CurrentHandle = NULL;
  this->EdgePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->EdgePicker->GetPath();
  if ( path != NULL )
  {
    this->LastPicker = this->EdgePicker;
    this->ValidPick = 1;

    this->CurrentHandle = 
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (this->CurrentHandle == this->EdgeActor[LEFT])
    {
      this->InteractionState = vtkRectangularRepresentation::MoveLeft;
    }
    else if (this->CurrentHandle == this->EdgeActor[RIGHT])
    {
      this->InteractionState = vtkRectangularRepresentation::MoveRight;
    } 
    else if (this->CurrentHandle == this->EdgeActor[TOP])
    {
      this->InteractionState = vtkRectangularRepresentation::MoveTop;
    } 
    else if (this->CurrentHandle == this->EdgeActor[BOTTOM])
    {
      this->InteractionState = vtkRectangularRepresentation::MoveBottom;
    }
  }
  else
  {
    this->RegionPicker->Pick(X,Y,0.0,this->Renderer);
    path = this->RegionPicker->GetPath();
    if (path != NULL)
    {
      this->LastPicker = this->RegionPicker;
      this->ValidPick = 1;

      this->CurrentHandle =
        reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
      if (this->CurrentHandle == RegionActor)
	this->InteractionState = vtkRectangularRepresentation::Translating;
      else
	assert(false);
    } else
    {
      this->InteractionState = vtkRectangularRepresentation::Outside;
    }
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkRectangularRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = ( state < vtkRectangularRepresentation::Outside ? vtkRectangularRepresentation::Outside :
            (state > vtkRectangularRepresentation::Scaling ? vtkRectangularRepresentation::Scaling : state) );

  // Depending on state, highlight appropriate parts of representation
//   int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkRectangularRepresentation::MoveLeft:
    case vtkRectangularRepresentation::MoveRight:
    case vtkRectangularRepresentation::MoveTop:
    case vtkRectangularRepresentation::MoveBottom:
      this->HighlightEdge(this->CurrentHandle);
      break;
    case vtkRectangularRepresentation::Translating:
    case vtkRectangularRepresentation::Scaling:
      this->HighlightOutline(1);
//       this->HighlightFace(-1);
      break;
    default:
      this->HighlightOutline(0);
      this->HighlightEdge(NULL);
//       this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkRectangularRepresentation::GetBounds()
{
  this->BuildRepresentation();
  return InitialBounds;
  this->BoundingBox->SetBounds(InitialBounds);
  return this->BoundingBox->GetBounds();
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::BuildRepresentation()
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
void vtkRectangularRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  for (unsigned int i=0; i<4; i++)
    this->EdgeActor[i]->ReleaseGraphicsResources(w);
  this->RegionActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------------
int vtkRectangularRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (unsigned int i=0; i<4; i++)
    count += this->EdgeActor[i]->RenderOpaqueGeometry(v);
  count += this->RegionActor->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (unsigned int i=0; i<4; i++)
    count += this->EdgeActor[i]->RenderTranslucentPolygonalGeometry(v);
  count += this->RegionActor->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  for (unsigned int i=0; i<4; i++)
    result |= this->EdgeActor[i]->HasTranslucentPolygonalGeometry();
  result |= this->RegionActor->HasTranslucentPolygonalGeometry();

//   // If the face is not selected, we are not really rendering translucent faces,
//   // hence don't bother taking it's opacity into consideration.
//   // Look at BUG #7301.
//   if (this->HexFace->GetProperty() == this->SelectedFaceProperty)
//     {
//     result |= this->HexFace->HasTranslucentPolygonalGeometry();
//     }

  return result;
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::HighlightEdge(vtkActor* actor)
{
  for(unsigned char i=0; i<4; i++)
  {
    if (this->EdgeActor[i] == actor)
      this->EdgeActor[i]->SetProperty(this->SelectedEdgeProperty);
    else
      this->EdgeActor[i]->SetProperty(this->EdgeProperty);
  }
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::HighlightOutline(int highlight)
{
//   if ( highlight )
//     {
//     this->RegionActor->SetProperty(this->SelectedOutlineProperty);
//     this->MarginActor[TOP]->SetProperty(this->SelectedOutlineProperty);
// //     this->BoundingFaceActor->SetProperty(this->SelectedOutlineProperty);
//     }
//   else
//     {
//     this->RegionActor->SetProperty(this->InclusionProperty);
//     this->MarginActor[TOP]->SetProperty(this->InclusionProperty);
// //     this->BoundingFaceActor->SetProperty(this->BoundingFaceProperty);
//     }
}

//----------------------------------------------------------------------------
void vtkRectangularRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";

  if ( this->SelectedEdgeProperty )
    {
    os << indent << "Selected Outline Property: " 
       << this->SelectedEdgeProperty << "\n";
    }
  else
    {
    os << indent << "Selected Outline Property: (none)\n";
    }

}

