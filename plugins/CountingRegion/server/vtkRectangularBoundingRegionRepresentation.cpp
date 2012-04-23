#include "vtkRectangularBoundingRegionRepresentation.h"

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

vtkStandardNewMacro(vtkRectangularBoundingRegionRepresentation);

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionRepresentation::vtkRectangularBoundingRegionRepresentation()
: Plane(vtkPVSliceView::AXIAL)
, Region(NULL)
, Slice(0)
, Init(false)
, NumPoints(0)
, NumSlices(0)
{
  // The initial state
  this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;

  memset(this->InclusionOffset, 0, 3*sizeof(double));
  memset(this->ExclusionOffset, 0, 3*sizeof(double));
  memset(this->Shift, 0, 4*sizeof(double));

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
vtkRectangularBoundingRegionRepresentation::~vtkRectangularBoundingRegionRepresentation()
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
void vtkRectangularBoundingRegionRepresentation::GetPolyData(vtkPolyData *pd)
{
  Q_ASSERT(false);
//   pd->SetPoints(this->RegionPolyData->GetPoints());
//   pd->SetPolys(this->RegionPolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::reset()
{
//   std::cout << "Shift's been reset" << std::endl;
  memset(this->Shift, 0, 4*sizeof(double));
  CreateRegion();
}


//----------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::StartWidgetInteraction(double e[2])
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
void vtkRectangularBoundingRegionRepresentation::WidgetInteraction(double e[2])
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
  if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveLeft )
  {
    this->MoveLeftEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveRight )
  {
    this->MoveRightEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveTop )
  {
    this->MoveTopEdge(prevPickPoint,pickPoint);
  }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveBottom )
  {
    this->MoveBottomEdge(prevPickPoint,pickPoint);
  }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveLeftEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  bool crossRightEdge = leftEdge() + shift + MIN_SLICE_SPACING >= rightEdge();
  bool validOffset = vtkPVSliceView::SAGITTAL != Plane
                  || InclusionOffset[hCoord()] + shift >= 0;
  if (!crossRightEdge && validOffset)
  {
    Shift[LEFT] += shift;
    InclusionOffset[hCoord()] += shift;
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveRightEdge(double* p1, double* p2)
{
  double shift = p2[hCoord()] - p1[hCoord()];
  bool crossLeftEdge = leftEdge() + MIN_SLICE_SPACING >= rightEdge() + shift;
  bool validOffset = vtkPVSliceView::SAGITTAL != Plane
                  || ExclusionOffset[hCoord()] - shift >= 0;
  if (!crossLeftEdge && validOffset)
  {
    Shift[RIGHT] += shift;
    ExclusionOffset[hCoord()] -= shift;
    UpdateRegion();
  }
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveTopEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  double crossBottomEdge = topEdge() + shift + MIN_SLICE_SPACING >= bottomEdge();
  bool validOffset = vtkPVSliceView::CORONAL != Plane
                  || InclusionOffset[vCoord()] + shift >= 0;
  if (!crossBottomEdge && validOffset)
  {
    Shift[TOP] += shift;
    InclusionOffset[vCoord()] += shift;
    UpdateRegion();
  }
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveBottomEdge(double* p1, double* p2)
{
  double shift = p2[vCoord()] - p1[vCoord()];
  double crossTopEdge = topEdge() + MIN_SLICE_SPACING >= bottomEdge() + shift;
  bool validOffset = vtkPVSliceView::CORONAL != Plane
                  || ExclusionOffset[vCoord()] - shift >= 0;
  if (!crossTopEdge && validOffset)
  {
    Shift[BOTTOM] += shift;
    ExclusionOffset[vCoord()] -= shift;
    UpdateRegion();
  }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateDefaultProperties()
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
int vtkRectangularBoundingRegionRepresentation::sliceNumber(double pos,
				vtkPVSliceView::VIEW_PLANE plane) const
{
  double point[3];
  for (int number = 0; number < NumSlices; number++)
  {
    this->Region->GetOutput()->GetPoints()->GetPoint(4*number, point);
//     this->Region->GetOutput()->GetPoints()->GetPoint(4*(number+1), next);
//     if (point[Plane] <= pos && pos < next[Plane])
    if (pos <= point[plane])
      return number;
  }
  return NumSlices - 1;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateRegion()
{
  switch (Plane)
  {
    case vtkPVSliceView::AXIAL:
      CreateXYFace();
      break;
    case vtkPVSliceView::SAGITTAL:
      CreateYZFace();
      break;
    case vtkPVSliceView::CORONAL:
      CreateXZFace();
      break;
  };
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::UpdateRegion()
{
  switch (Plane)
  {
    case vtkPVSliceView::AXIAL:
      UpdateXYFace();
      break;
    case vtkPVSliceView::SAGITTAL:
      CreateYZFace();
      break;
    case vtkPVSliceView::CORONAL:
      CreateXZFace();
      break;
  };
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXYFace()
{
  Region->UpdateWholeExtent();

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

  UpdateXYFace();
}


double slope(double p1[2], double p2[2])
{
  return (p2[1] - p1[1])/(p2[0] - p1[0]);
}
double interpolate(double p1[2], double p2[2], double x)
{
  double m = slope(p1, p2);
  return m*x + p1[1] - m*p1[0];
}

void intersection(double A1[2], double A2[2], double B1[2], double B2[2], double p[2])
{
  double mA = slope(A1, A2);
  double bA = A1[1] / (A1[0]*mA);
  double mB = slope(B1, B2);
  double bB = B1[1] / (B1[0]*mB);
  p[0] = (bB - bA)/(mA - mB);
  p[1] = mA*A1[0] + bA;
}

void vtkRectangularBoundingRegionRepresentation::UpdateXYFace()
{
  double LB[3], LT[3], RT[3], RB[3];

  // Get original Region Points
  Region->GetOutput()->GetPoint(Slice*4+3, LB);
  Region->GetOutput()->GetPoint(Slice*4+0, LT);
  Region->GetOutput()->GetPoint(Slice*4+1, RT);
  Region->GetOutput()->GetPoint(Slice*4+2, RB);

  // Change its depth to be always on top of the XY plane
  // according to Espina's Camera
  LB[2] = LT[2] = RT[2] = RB[2] = -0.1;

  // Shift edges' points
  LB[0] += Shift[LEFT];
  LT[0] += Shift[LEFT];

  LT[1] += Shift[TOP];
  RT[1] += Shift[TOP];

  RB[0] += Shift[RIGHT];
  RT[0] += Shift[RIGHT];

  RB[1] += Shift[BOTTOM];
  LB[1] += Shift[BOTTOM];

  this->Vertex->SetPoint(0, LB);
  this->Vertex->SetPoint(1, LT);
  this->Vertex->SetPoint(2, RT);
  this->Vertex->SetPoint(3, RB);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  BuildRepresentation();
}


//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateYZFace()
{
//   std::cout << "Created YZ FACE" << std::endl;
  double LB[3], RB[3];
  this->Region->GetOutput()->GetPoint(2, LB);
  this->Region->GetOutput()->GetPoint(NumPoints-2, RB);

//   std::cout << "LB: " << LB[2] << std::endl;
//   std::cout << "RB: " << RB[2] << std::endl;
//   std::cout << "LB+Shift: " << LB[2] + Shift[LEFT]  << std::endl;
//   std::cout << "RB+Shift: " << RB[2]+ Shift[RIGHT] << std::endl;
  int UpperSlice = sliceNumber(LB[2] + Shift[LEFT], vtkPVSliceView::AXIAL);
  int LowerSlice = sliceNumber(RB[2] + Shift[RIGHT], vtkPVSliceView::AXIAL);
//   std::cout << "Upper Slice: " << UpperSlice << std::endl;
//   std::cout << "Lower Slice: " << LowerSlice << std::endl;

  int numRepSlices = LowerSlice - UpperSlice + 1;
  if (numRepSlices == 0)
    return;

  unsigned int numIntervals = numRepSlices - 1;
  unsigned int numVertex    = numRepSlices * 2;

  // Set of point pairs. Each pair belong to the same slice .
  // First pair belongs to the left edge
  // Last pair belongs to the right edge
  // Odd indexed points belong to the top edge
  // Pair indexed points belong to the bottom edge
  /*
   *   1\ /5-7\ /11
   *   | 3     9 |
   *   |         |
   *   | 2-4     |
   *   0/   \6-8-10
   */
  this->Vertex->SetNumberOfPoints(numVertex);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->GetLines()->Reset();

  this->EdgePolyData[LEFT]->GetLines()->Allocate(
    this->EdgePolyData[LEFT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[LEFT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(0);
  this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(1);

  this->EdgePolyData[TOP]->GetLines()->Allocate(
    this->EdgePolyData[TOP]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[TOP]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(2*interval+1);
    this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(2*interval+3);
  }

  this->EdgePolyData[RIGHT]->GetLines()->Allocate(
    this->EdgePolyData[RIGHT]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(numVertex-2);
  this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(numVertex-1);

  this->EdgePolyData[BOTTOM]->GetLines()->Allocate(
    this->EdgePolyData[BOTTOM]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(2*interval);
    this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(2*interval+2);
  }

  double point[3];
  /// Loop over slices and create Top/Bottom Edges
  for (int slice=UpperSlice; slice <= LowerSlice; slice++)
  {
    int interval = slice - UpperSlice;
    // Bottom
    Region->GetOutput()->GetPoint(slice*4+3,point);
    point[0] = -0.1;
    point[1] += Shift[BOTTOM];
    if (slice == 0)
      point[2] += Shift[LEFT];
    else if (slice == NumSlices - 1)
      point[2] += Shift[RIGHT];
    this->Vertex->SetPoint(2*interval, point);
    // Top
    Region->GetOutput()->GetPoint(slice*4+0,point);
    point[0] = -0.1;
    point[1] += Shift[TOP];
    if (slice == 0)
      point[2] += Shift[LEFT];
    else if (slice == NumSlices - 1)
      point[2] += Shift[RIGHT];
    this->Vertex->SetPoint(2*interval+1, point);
  }

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXZFace()
{
  double LT[3], LB[3];
  this->Region->GetOutput()->GetPoint(0, LT);
  this->Region->GetOutput()->GetPoint(NumPoints-1, LB);

//   std::cout << "LT: " << LT[2] << std::endl;
//   std::cout << "LB: " << LB[2] << std::endl;
  int UpperSlice = sliceNumber(LT[2] + Shift[TOP], vtkPVSliceView::AXIAL);
  int LowerSlice = sliceNumber(LB[2] + Shift[BOTTOM], vtkPVSliceView::AXIAL);
//   std::cout << "Upper Slice: " << UpperSlice << std::endl;
//   std::cout << "Lower Slice: " << LowerSlice << std::endl;

  int numRepSlices = LowerSlice - UpperSlice + 1;
  if (numRepSlices == 0)
    return;

  unsigned int numIntervals = numRepSlices - 1;
  unsigned int numVertex    = numRepSlices * 2;

  // Set of point pairs. Each pair belong to the same slice .
  // First pair belongs to the top edge
  // Last pair belongs to the bottom edge
  // Pair indexed points belong to the left edge
  // Odd indexed points belong to the right edge
  /*
   *   0---------1
   *    \       /
   *     2     3
   *    /       \
   *   4         5
   *   |         |
   *   6---------7
   */
  this->Vertex->SetNumberOfPoints(numVertex);

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->GetLines()->Reset();

  this->EdgePolyData[LEFT]->GetLines()->Allocate(
    this->EdgePolyData[LEFT]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[LEFT]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(2*interval);
    this->EdgePolyData[LEFT]->GetLines()->InsertCellPoint(2*interval+2);
  }

  this->EdgePolyData[TOP]->GetLines()->Allocate(
    this->EdgePolyData[TOP]->GetLines()->EstimateSize(numIntervals,2));
  this->EdgePolyData[TOP]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(0);
  this->EdgePolyData[TOP]->GetLines()->InsertCellPoint(1);

  this->EdgePolyData[RIGHT]->GetLines()->Allocate(
    this->EdgePolyData[RIGHT]->GetLines()->EstimateSize(numIntervals,2));
  for(unsigned int interval=0; interval < numIntervals; interval++)
  {
    this->EdgePolyData[RIGHT]->GetLines()->InsertNextCell(2);
    this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(2*interval+1);
    this->EdgePolyData[RIGHT]->GetLines()->InsertCellPoint(2*interval+3);
  }

  this->EdgePolyData[BOTTOM]->GetLines()->Allocate(
    this->EdgePolyData[BOTTOM]->GetLines()->EstimateSize(1,2));
  this->EdgePolyData[BOTTOM]->GetLines()->InsertNextCell(2);
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(numVertex-2);
  this->EdgePolyData[BOTTOM]->GetLines()->InsertCellPoint(numVertex-1);

  double point[3];
  /// Loop over slices and create Top/Bottom Edges
  for ( int slice=UpperSlice; slice <= LowerSlice; slice++)
  {
    int interval = slice - UpperSlice;
    // LEFT
    Region->GetOutput()->GetPoint(slice*4+0,point);
    point[0] += Shift[LEFT];
    point[1] = -0.1;
    if (slice == 0)
      point[2] += Shift[TOP];
    else if (slice == NumSlices -1)
      point[2] += Shift[BOTTOM];
    this->Vertex->SetPoint(2*interval+0, point);
    //RIGHT
    Region->GetOutput()->GetPoint(slice*4+1,point);
    point[0] += Shift[RIGHT];
    point[1] = -0.1;
    if (slice == 0)
      point[2] += Shift[TOP];
    else if (slice == NumSlices -1)
      point[2] += Shift[BOTTOM];
    this->Vertex->SetPoint(2*interval+1, point);
  }

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetPlane(vtkPVSliceView::VIEW_PLANE plane)
{
  if (Plane == plane && Init)
    return;

  Init = true;
  Plane = plane;

//   double bounds[6];
//   BoundingBox->GetBounds(bounds);

  CreateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetSlice(double pos)
{
//   std::cout << "Plane: " << Plane << ", Slice: " << pos << /*", Spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2] <<*/ std::endl;
  Slice = sliceNumber(pos, Plane);
  UpdateRegion();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetRegion(vtkPolyDataAlgorithm *region)
{
  Region = region;
  this->Region->Update();
  this->Region->UpdateWholeExtent();
  this->NumPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  this->NumSlices = this->NumPoints / 4;
  this->NumVertex = this->NumSlices * 2;

  CreateRegion();
//   RegionPolyData->SetPoints(region->GetOutput()->GetPoints());
//   RegionPolyData->SetPolys(region->GetOutput()->GetPolys());
//   RegionPolyData->GetCellData()->SetScalars(region->GetOutput()->GetCellData()->GetScalars("Type"));
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::PlaceWidget(double bds[6])
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
int vtkRectangularBoundingRegionRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;
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
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveLeft;
    }
    else if (this->CurrentEdge == this->EdgeActor[RIGHT])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveRight;
    } 
    else if (this->CurrentEdge == this->EdgeActor[TOP])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveTop;
    } 
    else if (this->CurrentEdge == this->EdgeActor[BOTTOM])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveBottom;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = state < vtkRectangularBoundingRegionRepresentation::Outside ? vtkRectangularBoundingRegionRepresentation::Outside : state;
//   state = ( state < vtkRectangularBoundingRegionRepresentation::Outside ? vtkRectangularBoundingRegionRepresentation::Outside : 
//             (state > vtkRectangularBoundingRegionRepresentation::Scaling ? vtkRectangularBoundingRegionRepresentation::Scaling : state) );

  // Depending on state, highlight appropriate parts of representation
  this->InteractionState = state;
  switch (state)
    {
    case vtkRectangularBoundingRegionRepresentation::MoveLeft:
    case vtkRectangularBoundingRegionRepresentation::MoveRight:
    case vtkRectangularBoundingRegionRepresentation::MoveTop:
    case vtkRectangularBoundingRegionRepresentation::MoveBottom:
      this->HighlightEdge(this->CurrentEdge);
      break;
    case vtkRectangularBoundingRegionRepresentation::Translating:
      this->HighlightEdge(this->CurrentEdge);
      //       this->HighlightFace(-1);
      break;
    default:
//       this->HighlightOutline(0);
      this->HighlightEdge(NULL);
//       this->HighlightHandle(NULL);
//       this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkRectangularBoundingRegionRepresentation::GetBounds()
{
  this->BuildRepresentation();
  return Region->GetOutput()->GetBounds();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::BuildRepresentation()
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
void vtkRectangularBoundingRegionRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgeActor[i]->ReleaseGraphicsResources(w);
//   this->BoundingFaceActor->ReleaseGraphicsResources(w);
//   this->HexFace->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    count += this->EdgeActor[i]->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  for (EDGE i=LEFT; i <= BOTTOM; i = EDGE(i+1))
    result |= this->EdgeActor[i]->HasTranslucentPolygonalGeometry();

  return result;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::HighlightEdge(vtkActor* actor)
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
// void vtkRectangularBoundingRegionRepresentation::HighlightFace(int cellId)
// {
//   if ( cellId >= 0 )
//     {
//     vtkIdType npts;
//     vtkIdType *pts;
//     vtkCellArray *cells = this->HexFacePolyData->GetPolys();
//     this->InclusionPolyData->GetCellPoints(cellId, npts, pts);
//     this->HexFacePolyData->Modified();
//     cells->ReplaceCell(0,npts,pts);
//     this->CurrentHexFace = cellId;
//     this->HexFace->SetProperty(this->SelectedFaceProperty);
//     if ( !this->CurrentEdge )
//       {
//       this->CurrentEdge = this->HexFace;
//       }
//     }
//   else
//     {
//     this->HexFace->SetProperty(this->FaceProperty);
//     this->CurrentHexFace = -1;
//     }
// }

// //----------------------------------------------------------------------------
// void vtkRectangularBoundingRegionRepresentation::HighlightOutline(int highlight)
// {
//   if ( highlight )
//     {
//     this->EdgeActor[TOP]->SetProperty(this->SelectedOutlineProperty);
// //     this->BoundingFaceActor->SetProperty(this->SelectedOutlineProperty);
//     }
//   else
//     {
//     this->EdgeActor[TOP]->SetProperty(this->InclusionProperty);
// //     this->BoundingFaceActor->SetProperty(this->BoundingFaceProperty);
//     }
// }

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";
}