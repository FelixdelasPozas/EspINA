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


vtkStandardNewMacro(vtkRectangularBoundingRegionRepresentation);

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionRepresentation::vtkRectangularBoundingRegionRepresentation()
: Plane(vtkPVSliceView::AXIAL)
, Slice(0)
, Region(NULL)
, Init(false)
{
  // The initial state
  this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;

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
  this->BoundingBox = vtkBox::New();
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

  this->BoundingBox->Delete();

  this->InclusionEdgeProperty->Delete();
  this->ExclusionEdgeProperty->Delete();
  this->SelectedEdgeProperty->Delete();
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
//   CreateRegion();
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
  double *pts =
    static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);

  const int X = 0;

  pts[3*3 + X] = p2[X];
  pts[3*0 + X] = p2[X];

  InclusionOffset[X] += p2[X] - m_lastInclusionEdge[X];
  m_lastInclusionEdge[X] = p2[X];
  std::cout << "New Left Inclusion Offset " << InclusionOffset[X] << std::endl;

  this->EdgePolyData[LEFT]->Modified();
  this->EdgePolyData[TOP]->Modified();
  this->EdgePolyData[BOTTOM]->Modified();
  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveRightEdge(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   const int X = 0;
// 
//   assert(ViewType == XY || ViewType == XZ);
//   int contBorder1 = ViewType == XY?TOP:UPPER;
//   int contBorder2 = ViewType == XY?BOTTOM:LOWER;
//   
//   if (ViewType == XY)
//   {
//     pts[3*1 + X] = p2[X];
//     pts[3*2 + X] = p2[X];
//   }else if (ViewType == XZ)
//   {
//     double shift = (p2[X] - p1[X]);
//     for (int s=0; s < m_numSlices; s++)
//     {
//       pts[3*(2*s+1) + X] += shift;
//     }
//   }
// 
//   std::cout << "Old Right Exclusion Offset " << ExclusionOffset[X] << std::endl;
//   std::cout << "Old Right Exclusion Edge " << m_lastExclusionEdge[X] << std::endl;
//   std::cout << "Mouse Position " << p2[X] << std::endl;
//   ExclusionOffset[X] += m_lastExclusionEdge[X] - p2[X];
//   m_lastExclusionEdge[X] = p2[X];
//   std::cout << "New Right Exclusion Offset " << ExclusionOffset[X] << std::endl;
// 
//   this->EdgePolyData[RIGHT]->Modified();
//   this->EdgePolyData[contBorder1]->Modified();
//   this->EdgePolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveTopEdge(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   const int Y = 1;
// 
//   assert(ViewType == XY || ViewType == YZ);
//   int contBorder1 = ViewType == XY?LEFT:UPPER;
//   int contBorder2 = ViewType == XY?RIGHT:LOWER;
// 
//   if (ViewType == XY)
//   {
//     pts[3*0 + Y] = p2[Y]; 
//     pts[3*1 + Y] = p2[Y]; 
//   }else
//   {
//     double shift = (p2[Y] - p1[Y]);
//     for (int s=0; s < m_numSlices; s++)
//     {
//       pts[3*(2*s+1) + Y] += shift;
//     }
//   }
// 
//   InclusionOffset[Y] += p2[Y] - m_lastInclusionEdge[Y];
//   m_lastInclusionEdge[Y] = p2[Y];
//   std::cout << "New Top Inclusion Offset " << InclusionOffset[Y] << std::endl;
// 
//   this->EdgePolyData[TOP]->Modified();
//   this->EdgePolyData[contBorder1]->Modified();
//   this->EdgePolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveBottomEdge(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   const int Y = 1;
// 
//   assert(ViewType == XY || ViewType == YZ);
//   int contBorder1 = ViewType == XY?LEFT:UPPER;
//   int contBorder2 = ViewType == XY?RIGHT:LOWER;
// 
//   if (ViewType == XY)
//   {
//     pts[3*2 + Y] = p2[Y]; 
//     pts[3*3 + Y] = p2[Y]; 
//   }else if (ViewType == YZ)
//   {
//     double shift = (p2[Y] - p1[Y]);
//     for (int s=0; s < m_numSlices; s++)
//     {
//       pts[3*2*s + Y] += shift;
//     }
//   }else
//     assert(false);
// 
//   ExclusionOffset[Y] += m_lastExclusionEdge[Y] - p2[Y];
//   m_lastExclusionEdge[Y] = p2[Y];
//   std::cout << "New Botton Exclusion Offset " << ExclusionOffset[Y] << std::endl;
// 
//   this->EdgePolyData[BOTTOM]->Modified();
//   this->EdgePolyData[contBorder1]->Modified();
//   this->EdgePolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveUpperEdge(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   const int Z = 2;
// 
//   assert(ViewType == YZ || ViewType == XZ);
//   int contBorder1 = ViewType == YZ?TOP:LEFT;
//   int contBorder2 = ViewType == YZ?BOTTOM:RIGHT;
// 
//   pts[3*0 + Z] = p2[Z]; 
//   pts[3*1 + Z] = p2[Z]; 
// 
//   InclusionOffset[Z] += p2[Z] - m_lastInclusionEdge[Z];
//   m_lastInclusionEdge[Z] = p2[Z];
//   std::cout << "New Upper Inclusion Offset " << InclusionOffset[Z] << std::endl;
// 
//   this->EdgePolyData[UPPER]->Modified();
//   this->EdgePolyData[contBorder1]->Modified();
//   this->EdgePolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveLowerEdge(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   const int Z = 2;
// 
//   assert(ViewType == YZ || ViewType == XZ);
//   int contBorder1 = ViewType == YZ?TOP:LEFT;
//   int contBorder2 = ViewType == YZ?BOTTOM:RIGHT;
// 
//   int lastSlice = m_numSlices - 1;
//   int lastEdgePoint = lastSlice*2;
//   pts[3*(lastEdgePoint) + Z] = p2[Z];
//   pts[3*(lastEdgePoint+1) + Z] = p2[Z];
// 
//   ExclusionOffset[Z] += m_lastExclusionEdge[Z] - p2[Z];
//   m_lastExclusionEdge[Z] = p2[Z];
//   std::cout << "New Lower Exclusion Offset " << ExclusionOffset[Z] << std::endl;
// 
//   this->EdgePolyData[LOWER]->Modified();
//   this->EdgePolyData[contBorder1]->Modified();
//   this->EdgePolyData[contBorder2]->Modified();
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
  this->SelectedEdgeProperty = vtkProperty::New();
  this->SelectedEdgeProperty->SetRepresentationToSurface();
  this->SelectedEdgeProperty->SetOpacity(1.0);
  this->SelectedEdgeProperty->SetDiffuse(1.0);
  this->SelectedEdgeProperty->SetLineWidth(2.0);

  this->InvisibleProperty = vtkProperty::New();
  this->InvisibleProperty->SetRepresentationToWireframe();
  this->InvisibleProperty->SetAmbient(0.0);
  this->InvisibleProperty->SetDiffuse(0.0);
  this->InvisibleProperty->SetOpacity(0);
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
      UpdateYZFace();
      break;
    case vtkPVSliceView::CORONAL:
      UpdateXZFace();
      break;
  };
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXYFace()
{
//   std::cout << "Created XY FACE" << std::endl;
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

void vtkRectangularBoundingRegionRepresentation::UpdateXYFace()
{
  m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Edge
  m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Edge
  m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Edge
  m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Edge

  double point[3];
  // We need it to get lower limits
  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();

  this->Region->GetOutput()->GetPoint(numPoints-1,point);
  m_lastExclusionEdge[2] = point[2];

  // Point 0
  Region->GetOutput()->GetPoint(Slice*4+3,point);
  point[2] = -0.1;
  this->Vertex->SetPoint(0, point);
  m_lastInclusionEdge[0] = point[0];
  m_lastExclusionEdge[1] = point[1];

  // Point 1
  Region->GetOutput()->GetPoint(Slice*4+0,point);
  point[2] = -0.1;
  this->Vertex->SetPoint(1, point);

  //Point 2
  Region->GetOutput()->GetPoint(Slice*4+1,point);
  point[2] = -0.1;
  this->Vertex->SetPoint(2, point);
  m_lastInclusionEdge[1] = point[1];
  m_lastInclusionEdge[2] = point[2];

  // Point 3
  Region->GetOutput()->GetPoint(Slice*4+2,point);
  point[2] = -0.1;
  this->Vertex->SetPoint(3, point);
  m_lastExclusionEdge[0] = point[0];

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  BuildRepresentation();
}


//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateYZFace()
{
//   std::cout << "Created YZ FACE" << std::endl;
  this->Region->UpdateWholeExtent();

  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  unsigned int numSlices    = numPoints/4;
  unsigned int numIntervals = numSlices - 1;
  unsigned int numVertex    = numSlices * 2;

  m_numSlices = numSlices;

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

  UpdateYZFace();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::UpdateYZFace()
{
//   m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Edge
//   m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Edge
//   m_prevInclusion[2] = InclusionOffset[2]; // Use in Move Upper Edge
//   m_prevExclusion[2] = ExclusionOffset[2]; // Use in Move Lower Edge
  this->Region->UpdateWholeExtent();

  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  unsigned int numSlices = numPoints/4;
  int numLateralEdges = numSlices - 1;
  m_numSlices = numSlices;

  double point[3];

  // We need it to get right limits
  this->Region->GetOutput()->GetPoint(2,point);
  m_lastExclusionEdge[0] = point[0];
  for (unsigned int s=1; s < numSlices; s++)
  {
    this->Region->GetOutput()->GetPoint(s*4+2,point);
    m_lastExclusionEdge[0] = std::max(m_lastExclusionEdge[0], point[0]);
  }

//   // Point 0
//   this->Region->GetOutput()->GetPoint(0*4+3,point);
//   this->Vertex->SetPoint(0, point);
// 
//   // Point 1
//   this->Region->GetOutput()->GetPoint(0*4+0,point);
//   this->Vertex->SetPoint(1, point);
//   m_lastInclusionEdge[0] = point[0];
//   m_lastInclusionEdge[2] = point[2];

  /// Loop over slices and create Top/Bottom Edges
  for (unsigned int slice=0; slice< numSlices; slice++)
  {
//     int prevSlice = slice - 1;
    // Add new points
    // Point Slice_0
    Region->GetOutput()->GetPoint(slice*4+3,point);
//     point[0] = 200;
    this->Vertex->SetPoint(2*slice, point);//BOTTOM
    m_lastExclusionEdge[1] = point[1];
//     std::cout << "Slice: " << point[0] << " " << point[1]<< " " << point[2] << std::endl;

    // Point Slice_1
    Region->GetOutput()->GetPoint(slice*4+0,point);
//     point[0] = 200;
    this->Vertex->SetPoint(2*slice+1, point);//TOP
    m_lastInclusionEdge[1] = point[1];
    m_lastExclusionEdge[2] = point[2];
//     std::cout << "Slice: " << point[0] << " " << point[1]<< " " << point[2] << std::endl;
  }

  for(EDGE i = LEFT; i <= BOTTOM; i = EDGE(i+1))
    this->EdgePolyData[i]->Modified();

  BuildRepresentation();
}


//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXZFace()
{
  std::cout << "Created XZ FACE" << std::endl;
  this->Region->UpdateWholeExtent();

  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  unsigned int numSlices    = numPoints/4;
  unsigned int numIntervals = numSlices - 1;
  unsigned int numVertex    = numSlices * 2;

  m_numSlices = numSlices;

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

  UpdateXZFace();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::UpdateXZFace()
{
//   m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Edge
//   m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Edge
//   m_prevInclusion[2] = InclusionOffset[2]; // Use in Move Upper Edge
//   m_prevExclusion[2] = ExclusionOffset[2]; // Use in Move Lower Edge
// 
  int numPoints = Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  unsigned int numSlices = numPoints/4;
  int numLateralEdges = numSlices - 1;
  m_numSlices = numSlices;

  double point[3];

  // We need it to get right limits
  this->Region->GetOutput()->GetPoint(0,point);
  m_lastInclusionEdge[1] = point[1];
  this->Region->GetOutput()->GetPoint(3,point);
  m_lastExclusionEdge[1] = point[1];
  for (unsigned int s=1; s < numSlices; s++)
  {
    this->Region->GetOutput()->GetPoint(s*4+0,point);
    m_lastInclusionEdge[1] = std::min(m_lastInclusionEdge[1], point[1]);
    this->Region->GetOutput()->GetPoint(s*4+3,point);
    m_lastExclusionEdge[1] = std::max(m_lastExclusionEdge[1], point[1]);
  }

//   // Point 0
//   Region->GetOutput()->GetPoint(0*4+0,point);
//   this->Vertex->SetPoint(0, point);

//   // Point 1
//   Region->GetOutput()->GetPoint(0*4+1,point);
//   this->Vertex->SetPoint(1, point);
//   m_lastInclusionEdge[2] = point[2];

  /// Loop over slices and create Top/Bottom Edges
  for (unsigned int slice=0; slice< numSlices; slice++)
  {
//     int prevSlice = slice - 1;

    // Add new points
    // Point Slice_0
    Region->GetOutput()->GetPoint(slice*4+0,point);
    this->Vertex->SetPoint(2*slice+0, point);//TOP
    m_lastInclusionEdge[0] = point[0];
    // Point Slice_1
    Region->GetOutput()->GetPoint(slice*4+1,point);
    this->Vertex->SetPoint(2*slice+1, point);//RIGHT
    m_lastExclusionEdge[0] = point[0];
    m_lastExclusionEdge[2] = point[2];
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

  double bounds[6];
  BoundingBox->GetBounds(bounds);

  CreateRegion();

}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetSlice(double pos)
{
//   std::cout << "Plane: " << Plane << ", Slice: " << pos << /*", Spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2] <<*/ std::endl;
//   Region->UpdateWholeExtent();
  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();

  double point[3], next[3];
  for (unsigned int slice = 0; slice < numPoints/4; slice++)
  {
    this->Region->GetOutput()->GetPoints()->GetPoint(4*slice, point);
    this->Region->GetOutput()->GetPoints()->GetPoint(4*(slice+1), next);
    if (point[Plane] <= pos && pos < next[Plane])
    {
      Slice = slice;
      UpdateRegion();
      break;
    }
  }
  std::cout << "Change to Slice: " << Slice << " because of " << pos << std::endl;
//   int normalDir = Plane;
//   int slicePosition = slice*spacing[normalDir];
//   bool showRegion = m_lastInclusionEdge[normalDir] <= slicePosition &&
// 		    slicePosition <= m_lastExclusionEdge[normalDir];
// //   std::cout << "Exclusion on " << normalDir << ": " << m_prevExclusionCoord[normalDir] << std::endl;
//   if (showRegion)
//   {
//     for (int i=0; i<6; i++)
//       EdgeActor[i]->SetProperty(InclusionProperty);
//   }else
//   {
//     for (int i=0; i<6; i++)
//       EdgeActor[i]->SetProperty(InvisibleProperty);
//     return;
//   }
//   
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->Vertex->GetData())->GetPointer(0);
// 
//   if (ViewType == XY)
//   {
//     double point[3];
//     for (int p=0; p<this->Vertex->GetNumberOfPoints(); p++)
//     {
//       int numFaces = Region->GetOutput()->GetPoints()->GetNumberOfPoints()/4;
//       int validSlice = numFaces==2?0:slice;
//       
//       Region->GetOutput()->GetPoint(validSlice*4+p,point);
//       
// //       pts[3*p+0] = (p==3||p==0)?Inclusion[0]:point[0];
//       pts[3*p+0] = point[0];
//       pts[3*p+1] = point[1];
//       pts[3*p+2] = point[2];
// // 	pts[3*p] = slice; //BUG: get real spacing
//     }
//   }else
//   {
//     int normalDirection;
//     if (ViewType == YZ)
//       normalDirection = 0;
//     else
//       normalDirection = 1;
//     for (int p=0; p<this->Vertex->GetNumberOfPoints(); p++)
//     {
//       pts[3*p+normalDirection] = slice*spacing[normalDirection];
//     }
//   }
//   for(unsigned int i=0; i<6; i++)
//     this->EdgePolyData[i]->Modified();
// //   std::cout << "View " << ViewType <<  " : " << BoundingFacePoints->GetNumberOfPoints() << std::endl;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetRegion(vtkPolyDataAlgorithm *region)
{
  Region = region;
  region->Update();
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
  //BUG: //TODO
  double bounds[6] = {0,6000,0,6000,0,6000};
  this->BoundingBox->SetBounds(bounds);
  return this->BoundingBox->GetBounds();
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
      this->EdgeActor[edge]->SetProperty(this->SelectedEdgeProperty);
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