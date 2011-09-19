#include "vtkRectangularBoundingRegionRepresentation.h"

#include "vtkActor.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkCallbackCommand.h"
#include "vtkBox.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkTransform.h"
#include "vtkDoubleArray.h"
#include "vtkBox.h"
#include "vtkPlanes.h"
#include "vtkCamera.h"
#include "vtkAssemblyPath.h"
#include "vtkWindow.h"
#include "vtkObjectFactory.h"
#include <vtkSmartPointer.h>
#include <vtkLine.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkLookupTable.h>


vtkStandardNewMacro(vtkRectangularBoundingRegionRepresentation);

enum View {XY,YZ,XZ,VOL};

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionRepresentation::vtkRectangularBoundingRegionRepresentation()
{
  // The initial state
  this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;
  this->ViewType = VOL; //Default 3D View
  this->Slice = 0;
  this->Region = NULL;

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Construct the poly data representing the bounding region
  this->InclusionPolyData = vtkPolyData::New();
  this->InclusionLUT = vtkLookupTable::New();
  this->InclusionLUT->SetNumberOfTableValues(2);
  this->InclusionLUT->Build();
  InclusionLUT->SetTableValue(0,1,0,0);
  InclusionLUT->SetTableValue(1,0,1,0);
  
  this->InclusionMapper = vtkPolyDataMapper::New();
  this->InclusionMapper->SetInput(InclusionPolyData);
  this->InclusionMapper->SetLookupTable(this->InclusionLUT);
  this->InclusionActor = vtkActor::New();
  this->InclusionActor->SetMapper(this->InclusionMapper);
  this->InclusionActor->SetProperty(this->InclusionProperty);

  // Construct the poly data representing a bounding face
  this->BoundingFacePolyData = vtkPolyData::New();
  this->BoundingFaceMapper = vtkPolyDataMapper::New();
  this->BoundingFaceMapper->SetInput(BoundingFacePolyData);
  this->BoundingFaceMapper->SetLookupTable(InclusionLUT);

  // Construct initial points
  this->Points = vtkPoints::New(VTK_DOUBLE);
  this->Points->SetNumberOfPoints(8);//8 corners;
  this->BoundingFacePoints = vtkPoints::New(VTK_DOUBLE);
  this->BoundingFacePoints->SetNumberOfPoints(4);
  this->InclusionPolyData->SetPoints(this->Points);
  this->BoundingFacePolyData->SetPoints(this->BoundingFacePoints);
  
  // Construct connectivity for the faces. These are used to perform
  // the picking.
  vtkIdType pts[4];
  vtkCellArray *inCells = vtkCellArray::New();
  inCells->Allocate(inCells->EstimateSize(3,4));
  vtkCellArray *exCells = vtkCellArray::New();
  exCells->Allocate(exCells->EstimateSize(3,4));
  pts[0] = 3; pts[1] = 0; pts[2] = 4; pts[3] = 7;
  exCells->InsertNextCell(4,pts);
  pts[0] = 1; pts[1] = 2; pts[2] = 6; pts[3] = 5;
  inCells->InsertNextCell(4,pts);
  pts[0] = 0; pts[1] = 1; pts[2] = 5; pts[3] = 4;
  inCells->InsertNextCell(4,pts);
  pts[0] = 2; pts[1] = 3; pts[2] = 7; pts[3] = 6;
  exCells->InsertNextCell(4,pts);
  pts[0] = 0; pts[1] = 3; pts[2] = 2; pts[3] = 1;
  inCells->InsertNextCell(4,pts);
  pts[0] = 4; pts[1] = 5; pts[2] = 6; pts[3] = 7;
  exCells->InsertNextCell(4,pts);
//   this->InclusionPolyData->SetPolys(inCells);
//   this->ExclusionPolyData->SetPolys(exCells);
  inCells->Delete();
  exCells->Delete();
//   this->InclusionPolyData->BuildCells();
//   this->BoundingFacePolyData->BuildCells();
  
//   // Construct connectivity for the bounding face (the one shown in planes).
//   //TODO: These are used to perform the picking
//   vtkCellArray *inLines = vtkCellArray::New();
//   inLines->Allocate(inLines->EstimateSize(4,2));
//   vtkSmartPointer<vtkIntArray> lineData = vtkSmartPointer<vtkIntArray>::New();
//   vtkSmartPointer<vtkLine> inLine0 = vtkSmartPointer<vtkLine>::New();
//   vtkSmartPointer<vtkLine> inLine1 = vtkSmartPointer<vtkLine>::New();
//   vtkSmartPointer<vtkLine> exLine0 = vtkSmartPointer<vtkLine>::New();
//   vtkSmartPointer<vtkLine> exLine1 = vtkSmartPointer<vtkLine>::New();
//   inLine0->GetPointIds()->SetId(0,0);
//   inLine0->GetPointIds()->SetId(1,3);
//   lineData->InsertNextValue(255);
//   inLine1->GetPointIds()->SetId(0,0);
//   inLine1->GetPointIds()->SetId(1,1);
//   lineData->InsertNextValue(255);
//   exLine0->GetPointIds()->SetId(0,1);
//   exLine0->GetPointIds()->SetId(1,2);
//   lineData->InsertNextValue(0);
//   exLine1->GetPointIds()->SetId(0,3);
//   exLine1->GetPointIds()->SetId(1,2);
//   lineData->InsertNextValue(0);
//   inLines->InsertNextCell(inLine0);
//   inLines->InsertNextCell(inLine1);
//   inLines->InsertNextCell(exLine0);
//   inLines->InsertNextCell(exLine1);
//   this->BoundingFacePolyData->SetLines(inLines);
//   this->BoundingFacePolyData->GetCellData()->SetScalars(lineData);
  
  // The face of the hexahedra
  vtkCellArray *cells = vtkCellArray::New();
  cells->Allocate(cells->EstimateSize(1,4));
  cells->InsertNextCell(4,pts); //temporary, replaced later
  this->HexFacePolyData = vtkPolyData::New();
  this->HexFacePolyData->SetPoints(this->Points);
  this->HexFacePolyData->SetPolys(cells);
  this->HexFaceMapper = vtkPolyDataMapper::New();
  this->HexFaceMapper->SetInput(HexFacePolyData);
  this->HexFace = vtkActor::New();
  this->HexFace->SetMapper(this->HexFaceMapper);
  this->HexFace->SetProperty(this->FaceProperty);
  cells->Delete();

  // Define the point coordinates
  double bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;
  // Points 8-14 are down by PositionHandles();
  this->BoundingBox = vtkBox::New();
  this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->HexPicker = vtkCellPicker::New();
  this->HexPicker->SetTolerance(0.001);
//   this->HexPicker->AddPickList(InclusionActor);
  this->HexPicker->PickFromListOn();
  
  this->CurrentHandle = NULL;

  // Internal data memebers for performance
  this->Transform = vtkTransform::New();
  this->PlanePoints = vtkPoints::New(VTK_DOUBLE);
  this->PlanePoints->SetNumberOfPoints(6);
  this->PlaneNormals = vtkDoubleArray::New();
  this->PlaneNormals->SetNumberOfComponents(3);
  this->PlaneNormals->SetNumberOfTuples(6);
  this->Matrix = vtkMatrix4x4::New();
}

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionRepresentation::~vtkRectangularBoundingRegionRepresentation()
{  
  this->InclusionActor->Delete();
  this->InclusionMapper->Delete();
  this->InclusionPolyData->Delete();
//   this->BoundingFaceActor->Delete();
  this->BoundingFaceMapper->Delete();
  this->BoundingFacePolyData->Delete();
  this->Points->Delete();

  this->HexFace->Delete();
  this->HexFaceMapper->Delete();
  this->HexFacePolyData->Delete();

  this->HexPicker->Delete();

  this->Transform->Delete();
  this->BoundingBox->Delete();
  this->PlanePoints->Delete();
  this->PlaneNormals->Delete();
  this->Matrix->Delete();
  
  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->InclusionProperty->Delete();
  this->SelectedOutlineProperty->Delete();
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::GetPolyData(vtkPolyData *pd)
{
  pd->SetPoints(this->InclusionPolyData->GetPoints());
  pd->SetPolys(this->InclusionPolyData->GetPolys());
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
  if ( this->LastPicker == this->HexPicker )
    {
    this->HexPicker->GetPickPosition(pos);
    }
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF0 )
    {
    this->MoveMinusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF1 )
    {
    this->MovePlusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF2 )
    {
    this->MoveMinusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF3 )
    {
    this->MovePlusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF4 )
    {
    this->MoveMinusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveF5 )
    {
    this->MovePlusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::Translating )
    {
    this->Translate(prevPickPoint, pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint, 
                static_cast<int>(e[0]), static_cast<int>(e[1]));
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::Rotating )
    {
//     this->Rotate(static_cast<int>(e[0]), static_cast<int>(e[1]), prevPickPoint, pickPoint, vpn);
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveFace(double *p1, double *p2, double *dir, 
                                    double *x1, double *x2, double *x3, double *x4,
                                    double *x5)
  {
  int i;
  double v[3], v2[3];

  for (i=0; i<3; i++)
    {
    v[i] = p2[i] - p1[i];
    v2[i] = dir[i];
    }

  vtkMath::Normalize(v2);
  double f = vtkMath::Dot(v,v2);
  
  for (i=0; i<3; i++)
    {
    v[i] = f*v2[i];
  
    x1[i] += v[i];
    x2[i] += v[i];
    x3[i] += v[i];
    x4[i] += v[i];
    x5[i] += v[i];
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::GetDirection(const double Nx[3],const double Ny[3], 
                                        const double Nz[3], double dir[3])
{
  double dotNy, dotNz;
  double y[3];

  if(vtkMath::Dot(Nx,Nx)!=0)
    {
    dir[0] = Nx[0];
    dir[1] = Nx[1];
    dir[2] = Nx[2];
    }
  else 
    {
    dotNy = vtkMath::Dot(Ny,Ny);
    dotNz = vtkMath::Dot(Nz,Nz);
    if(dotNy != 0 && dotNz != 0)
      {
      vtkMath::Cross(Ny,Nz,dir);
      }
    else if(dotNy != 0)
      {
      //dir must have been initialized to the 
      //corresponding coordinate direction before calling
      //this method
      vtkMath::Cross(Ny,dir,y);
      vtkMath::Cross(y,Ny,dir);
      }
    else if(dotNz != 0)
      {
      //dir must have been initialized to the 
      //corresponding coordinate direction before calling
      //this method
      vtkMath::Cross(Nz,dir,y);
      vtkMath::Cross(y,Nz,dir);
      }
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MovePlusXFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*9;

  double *x1 = pts + 3*1;
  double *x2 = pts + 3*2;
  double *x3 = pts + 3*5;
  double *x4 = pts + 3*6;
  
  double dir[3] = { 1 , 0 , 0};
  this->ComputeNormals();
  this->GetDirection(this->N[1],this->N[3],this->N[5],dir);
  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveMinusXFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*8;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*3;
  double *x3 = pts + 3*4;
  double *x4 = pts + 3*7;
  
  double dir[3]={-1,0,0};
  this->ComputeNormals();
  this->GetDirection(this->N[0],this->N[4],this->N[2],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MovePlusYFace(double *p1, double *p2)
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*11;

  double *x1 = pts + 3*2;
  double *x2 = pts + 3*3;
  double *x3 = pts + 3*6;
  double *x4 = pts + 3*7;
  
  double dir[3]={0,1,0};
  this->ComputeNormals();
  this->GetDirection(this->N[3],this->N[5],this->N[1],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveMinusYFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*10;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*1;
  double *x3 = pts + 3*4;
  double *x4 = pts + 3*5;

  double dir[3] = {0, -1, 0};
  this->ComputeNormals();
  this->GetDirection(this->N[2],this->N[0],this->N[4],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MovePlusZFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*13;

  double *x1 = pts + 3*4;
  double *x2 = pts + 3*5;
  double *x3 = pts + 3*6;
  double *x4 = pts + 3*7;

  double dir[3]={0,0,1};
  this->ComputeNormals();
  this->GetDirection(this->N[5],this->N[1],this->N[3],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveMinusZFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*12;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*1;
  double *x3 = pts + 3*2;
  double *x4 = pts + 3*3;

  double dir[3]={0,0,-1};
  this->ComputeNormals();
  this->GetDirection(this->N[4],this->N[2],this->N[0],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
// Loop through all points and translate them
void vtkRectangularBoundingRegionRepresentation::Translate(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double v[3];

  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];
  
  // Move the corners
  for (int i=0; i<8; i++)
    {
    *pts++ += v[0];
    *pts++ += v[1];
    *pts++ += v[2];
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::Scale(double *vtkNotUsed(p1),
                                 double *vtkNotUsed(p2),
                                 int vtkNotUsed(X),
                                 int Y)
{
  double *pts =
      static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *center 
    = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(3*14);
  double sf;

  if ( Y > this->LastEventPosition[1] )
    {
    sf = 1.03;
    }
  else
    {
    sf = 0.97;
    }
  
  // Move the corners
  for (int i=0; i<8; i++, pts+=3)
    {
    pts[0] = sf * (pts[0] - center[0]) + center[0];
    pts[1] = sf * (pts[1] - center[1]) + center[1];
    pts[2] = sf * (pts[2] - center[2]) + center[2];
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::ComputeNormals()
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *px = pts + 3*1;
  double *py = pts + 3*3;
  double *pz = pts + 3*4;
  int i;
  
  for (i=0; i<3; i++)
    {
    this->N[0][i] = p0[i] - px[i];
    this->N[2][i] = p0[i] - py[i];
    this->N[4][i] = p0[i] - pz[i];
    }
  vtkMath::Normalize(this->N[0]);
  vtkMath::Normalize(this->N[2]);
  vtkMath::Normalize(this->N[4]);
  for (i=0; i<3; i++)
    {
    this->N[1][i] = -this->N[0][i];
    this->N[3][i] = -this->N[2][i];
    this->N[5][i] = -this->N[4][i];
    }
}

  
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateDefaultProperties()
{
  // Face properties
  this->FaceProperty = vtkProperty::New();
  this->FaceProperty->SetColor(1,1,1);
  this->FaceProperty->SetOpacity(0.0);

  this->SelectedFaceProperty = vtkProperty::New();
  this->SelectedFaceProperty->SetColor(1,1,0);
  this->SelectedFaceProperty->SetOpacity(0.25);
  
  // Inclusive Outline properties
  this->InclusionProperty = vtkProperty::New();
  this->InclusionProperty->SetRepresentationToSurface();
  this->InclusionProperty->SetOpacity(0.7);
//   this->InclusionProperty->SetAmbient(1.0);
  this->InclusionProperty->SetDiffuse(1.0);
//   this->InclusionProperty->SetAmbientColor(0.0,1.0,0.0);
//   this->InclusionProperty->SetDiffuseColor(0.0,1.0,0.0);
  this->InclusionProperty->SetLineWidth(1.0);

  this->InvisibleProperty = vtkProperty::New();
  this->InvisibleProperty->SetRepresentationToWireframe();
  this->InvisibleProperty->SetAmbient(0.0);
  this->InvisibleProperty->SetDiffuse(0.0);
  this->InvisibleProperty->SetOpacity(0);

  this->SelectedOutlineProperty = vtkProperty::New();
  this->SelectedOutlineProperty->SetRepresentationToWireframe();
  this->SelectedOutlineProperty->SetAmbient(1.0);
  this->SelectedOutlineProperty->SetAmbientColor(0.0,1.0,0.0);
  this->SelectedOutlineProperty->SetLineWidth(2.0);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXYFace()
{
  std::cout << "Created XY FACE" << std::endl;
  Region->UpdateWholeExtent();
  
//   // Construct connectivity for the bounding face (the one shown in planes).
//   //TODO: These are used to perform the picking
  vtkCellArray *inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(4,2));
  vtkSmartPointer<vtkIntArray> lineData = vtkSmartPointer<vtkIntArray>::New();
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
  
  this->BoundingFacePoints->SetNumberOfPoints(4);
  
  double point[3];
  
  Region->GetOutput()->GetPoint(0*4+0,point);
  this->BoundingFacePoints->SetPoint(0, point);
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,3);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
  
  Region->GetOutput()->GetPoint(0*4+1,point);
  this->BoundingFacePoints->SetPoint(1, point);
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
  
  Region->GetOutput()->GetPoint(0*4+2,point);
  this->BoundingFacePoints->SetPoint(2, point);
  line->GetPointIds()->SetId(0,1);
  line->GetPointIds()->SetId(1,2);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
  
  Region->GetOutput()->GetPoint(0*4+3,point);
  this->BoundingFacePoints->SetPoint(3, point);
  line->GetPointIds()->SetId(0,3);
  line->GetPointIds()->SetId(1,2);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
  
  this->BoundingFacePolyData->SetLines(inLines);
  this->BoundingFacePolyData->GetCellData()->SetScalars(lineData);
  this->BoundingFacePolyData->Modified();
  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateYZFace()
{
  std::cout << "Created YZ FACE" << std::endl;
  Region->UpdateWholeExtent();
  
  int numPoints = Region->GetOutput()->GetPoints()->GetNumberOfPoints();  
  int numLines = numPoints/2;
  int numSlices = numPoints/4;
   
  vtkCellArray *inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(numLines,2));
  vtkSmartPointer<vtkIntArray> lineData = vtkSmartPointer<vtkIntArray>::New();
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
  //every 2 lines share 1 point, thus, only half are needed
//   this->BoundingFacePoints->SetNumberOfPoints(numLines);
  this->BoundingFacePoints->SetNumberOfPoints(numLines);
  
  double point[3], prevPoint[3];
  vtkIdType id = 0;
  
  // Create Min X Line
  Region->GetOutput()->GetPoint(0*4+0,point);
  this->BoundingFacePoints->SetPoint(id, point);
//   std::cout << "Point " << 0 << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
  Region->GetOutput()->GetPoint(0*4+3,point);
  this->BoundingFacePoints->SetPoint(++id, point);
//   std::cout << "Point " << 1 << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
//   std::cout << "Connecting point " << 0 << " with " << 1 << std::endl;
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
  
  // Loop over slices and create Top/Bottom Lines
  for (int f=1; f< numSlices; f++)
  {
    Region->GetOutput()->GetPoint(f*4+0,point);
    this->BoundingFacePoints->SetPoint(++id, point);
//     std::cout << "Connecting point " << id << " with " << (id-2) << std::endl;
//     std::cout << "Point " << id << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    line->GetPointIds()->SetId(0,id-2);
    line->GetPointIds()->SetId(1,id);
    inLines->InsertNextCell(line);
    lineData->InsertNextValue(255);
    Region->GetOutput()->GetPoint(f*4+3,point);
    this->BoundingFacePoints->SetPoint(++id, point);
//     std::cout << "Connecting point " << id << " with " << (id-2) << std::endl;
//     std::cout << "Point " << id << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    line->GetPointIds()->SetId(0,id-2);
    line->GetPointIds()->SetId(1,id);
    inLines->InsertNextCell(line);
    lineData->InsertNextValue(0);
  }

  // Create Max X Line
  line->GetPointIds()->SetId(0,id-1);
  line->GetPointIds()->SetId(1,id);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
  
  this->BoundingFacePolyData->SetLines(inLines);
  this->BoundingFacePolyData->GetCellData()->SetScalars(lineData);
  this->BoundingFacePolyData->Modified();
  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXZFace()
{
  std::cout << "Created XZ FACE" << std::endl;
  Region->UpdateWholeExtent();
  
  int numPoints = Region->GetOutput()->GetPoints()->GetNumberOfPoints();  
  int numLines = numPoints/2;
  int numSlices = numPoints/4;
   
  vtkCellArray *inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(numLines,2));
  vtkSmartPointer<vtkIntArray> lineData = vtkSmartPointer<vtkIntArray>::New();
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
  //every 2 lines share 1 point, thus, only half are needed
//   this->BoundingFacePoints->SetNumberOfPoints(numLines);
  this->BoundingFacePoints->SetNumberOfPoints(numLines);
  double point[3], prevPoint[3];
  vtkIdType id = 0;
  
  // Create Min X Line
  Region->GetOutput()->GetPoint(0*4+0,point);
  this->BoundingFacePoints->SetPoint(id, point);
//   std::cout << "Point " << 0 << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
  Region->GetOutput()->GetPoint(0*4+1,point);
  this->BoundingFacePoints->SetPoint(++id, point);
//   std::cout << "Point " << 1 << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
//   std::cout << "Connecting point " << 0 << " with " << 1 << std::endl;
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
  
  // Loop over slices and create Top/Bottom Lines
  for (int f=1; f< numSlices; f++)
  {
    Region->GetOutput()->GetPoint(f*4+0,point);
    this->BoundingFacePoints->SetPoint(++id, point);
//     std::cout << "Connecting point " << id << " with " << (id-2) << std::endl;
//     std::cout << "Point " << id << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    line->GetPointIds()->SetId(0,id-2);
    line->GetPointIds()->SetId(1,id);
    inLines->InsertNextCell(line);
    lineData->InsertNextValue(255);
    Region->GetOutput()->GetPoint(f*4+1,point);
    this->BoundingFacePoints->SetPoint(++id, point);
//     std::cout << "Connecting point " << id << " with " << (id-2) << std::endl;
//     std::cout << "Point " << id << ": " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    line->GetPointIds()->SetId(0,id-2);
    line->GetPointIds()->SetId(1,id);
    inLines->InsertNextCell(line);
    lineData->InsertNextValue(0);
  }

  // Create Max X Line
  line->GetPointIds()->SetId(0,id-1);
  line->GetPointIds()->SetId(1,id);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
  
  this->BoundingFacePolyData->SetLines(inLines);
  this->BoundingFacePolyData->GetCellData()->SetScalars(lineData);
  this->BoundingFacePolyData->Modified();
  BuildRepresentation();
}


//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetViewType(int type)
{
  if (ViewType == type)
    return;
    
  View viewType = (View)type;
  ViewType = viewType;
  
  double bounds[6];
  BoundingBox->GetBounds(bounds);
	  
  if (viewType == VOL)
  {
    assert(false);
    InclusionActor->SetMapper(InclusionMapper);
  }else
  {
      if (viewType == XY)
	CreateXYFace();
      else if (viewType == YZ)
	CreateYZFace();
      else if (viewType == XZ)
	CreateXZFace();
      else
	assert(false);
      InclusionProperty->SetOpacity(1.0);
      InclusionActor->SetMapper(BoundingFaceMapper);
  }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetSlice(int slice, double spacing)
{
  Slice = slice;
  spacing = spacing;
//   return;
  
  if (ViewType == VOL)
    return;
  
//   if (slice < 60)
//   {
//     InclusionActor->SetProperty(InvisibleProperty);
//     return;
//   }else
//   {
//     InclusionActor->SetProperty(InclusionProperty);
//   }
  
  double *pts =
    static_cast<vtkDoubleArray *>(this->BoundingFacePoints->GetData())->GetPointer(0);

  if (ViewType == XY)
  {
    double point[3];
    for (int p=0; p<this->BoundingFacePoints->GetNumberOfPoints(); p++)
    {
      int numFaces = Region->GetOutput()->GetPoints()->GetNumberOfPoints()/4;
      int validSlice = numFaces==2?0:slice;
      
      Region->GetOutput()->GetPoint(validSlice*4+p,point);
      
      pts[3*p+0] = point[0];
      pts[3*p+1] = point[1];
      pts[3*p+2] = point[2];
// 	BoundingFacePoints->SetPoint(p,point);
// 	pts[3*p] = slice; //BUG: get real spacing
    }
    
  }else
  {
    int normalDirection;
    if (ViewType == YZ)
      normalDirection = 0;
    else
      normalDirection = 1;
    for (int p=0; p<this->BoundingFacePoints->GetNumberOfPoints(); p++)
    {
      pts[3*p+normalDirection] = slice*spacing;
    }
  }
//   std::cout << "View " << ViewType <<  " : " << BoundingFacePoints->GetNumberOfPoints() << std::endl;
  BoundingFacePolyData->Modified();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetRegion(vtkPolyDataAlgorithm *region)
{
  Region = region;
  region->Update();
  InclusionPolyData->SetPoints(region->GetOutput()->GetPoints());
  InclusionPolyData->SetPolys(region->GetOutput()->GetPolys());
  InclusionPolyData->GetCellData()->SetScalars(region->GetOutput()->GetCellData()->GetScalars("Type"));
}


//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::PlaceWidget(double bds[6])
{
  std::cout << "Place Widget" << std::endl;
  int i;
  double bounds[6], center[3];
  
  this->AdjustBounds(bds,bounds,center);
  
  this->Points->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  this->Points->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  this->Points->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  this->Points->SetPoint(3, bounds[0], bounds[3], bounds[4]);
  this->Points->SetPoint(4, bounds[0], bounds[2], bounds[5]);
  this->Points->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  this->Points->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  this->Points->SetPoint(7, bounds[0], bounds[3], bounds[5]);

  this->BoundingFacePoints->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  this->BoundingFacePoints->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  this->BoundingFacePoints->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  this->BoundingFacePoints->SetPoint(3, bounds[0], bounds[3], bounds[4]);

  for (i=0; i<6; i++)
    {
    this->InitialBounds[i] = bounds[i];
    }
  this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

  this->ComputeNormals();
  this->ValidPick = 1; //since we have set up widget
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::GetTransform(vtkTransform *t)
{
  std::cout << "GetTransform" << std::endl;
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *p1 = pts + 3*1;
  double *p3 = pts + 3*3;
  double *p4 = pts + 3*4;
  double *p14 = pts + 3*14;
  double center[3], translate[3], scale[3], scaleVec[3][3];
  double InitialCenter[3];
  int i;

  // The transformation is relative to the initial bounds.
  // Initial bounds are set when PlaceWidget() is invoked.
  t->Identity();
  
  // Translation
  for (i=0; i<3; i++)
    {
    InitialCenter[i] = 
      (this->InitialBounds[2*i+1]+this->InitialBounds[2*i]) / 2.0;
    center[i] = p14[i] - InitialCenter[i];
    }
  translate[0] = center[0] + InitialCenter[0];
  translate[1] = center[1] + InitialCenter[1];
  translate[2] = center[2] + InitialCenter[2];
  t->Translate(translate[0], translate[1], translate[2]);
  
  // Orientation
  this->Matrix->Identity();
  this->ComputeNormals();
  for (i=0; i<3; i++)
    {
    this->Matrix->SetElement(i,0,this->N[1][i]);
    this->Matrix->SetElement(i,1,this->N[3][i]);
    this->Matrix->SetElement(i,2,this->N[5][i]);
    }
  t->Concatenate(this->Matrix);

  // Scale
  for (i=0; i<3; i++)
    {
    scaleVec[0][i] = (p1[i] - p0[i]);
    scaleVec[1][i] = (p3[i] - p0[i]);
    scaleVec[2][i] = (p4[i] - p0[i]);
    }

  scale[0] = vtkMath::Norm(scaleVec[0]);
  if (this->InitialBounds[1] != this->InitialBounds[0])
    {
    scale[0] = scale[0] / (this->InitialBounds[1]-this->InitialBounds[0]);
    }
  scale[1] = vtkMath::Norm(scaleVec[1]);
  if (this->InitialBounds[3] != this->InitialBounds[2])
    {
    scale[1] = scale[1] / (this->InitialBounds[3]-this->InitialBounds[2]);
    }
  scale[2] = vtkMath::Norm(scaleVec[2]);
  if (this->InitialBounds[5] != this->InitialBounds[4])
    {
    scale[2] = scale[2] / (this->InitialBounds[5]-this->InitialBounds[4]);
    }
  t->Scale(scale[0],scale[1],scale[2]);
  
  // Add back in the contribution due to non-origin center
  t->Translate(-InitialCenter[0], -InitialCenter[1], -InitialCenter[2]);
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetTransform(vtkTransform* t)
{
  std::cout << "SetTransform" << std::endl;
  if (!t)
    {
    vtkErrorMacro(<<"vtkTransform t must be non-NULL");
    return;
    }

  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double xIn[3];
  // make sure the transform is up-to-date before using it
  t->Update();

  // Position the eight points of the box and then update the
  // position of the other handles.
  double *bounds=this->InitialBounds;

  xIn[0] = bounds[0]; xIn[1] = bounds[2]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts);

  xIn[0] = bounds[1]; xIn[1]= bounds[2]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+3);

  xIn[0] = bounds[1]; xIn[1]= bounds[3]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+6);

  xIn[0] = bounds[0]; xIn[1]= bounds[3]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+9);

  xIn[0] = bounds[0]; xIn[1]= bounds[2]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+12);

  xIn[0] = bounds[1]; xIn[1]= bounds[2]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+15);

  xIn[0] = bounds[1]; xIn[1]= bounds[3]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+18);

  xIn[0] = bounds[0]; xIn[1]= bounds[3]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+21);

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
  
  return this->InteractionState;
  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = NULL;
  this->CurrentHandle = NULL;
  this->HexPicker->Pick(X,Y,0.0,this->Renderer);
  path = this->HexPicker->GetPath();
  if ( path != NULL )
  {
    this->LastPicker = this->HexPicker;
    this->ValidPick = 1;
    if ( !modify )
    {
//       this->InteractionState = vtkRectangularBoundingRegionRepresentation::Rotating;
    }
    else
    {
//       this->CurrentHandle = this->Handle[6];
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::Translating;
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
  state = ( state < vtkRectangularBoundingRegionRepresentation::Outside ? vtkRectangularBoundingRegionRepresentation::Outside : 
            (state > vtkRectangularBoundingRegionRepresentation::Scaling ? vtkRectangularBoundingRegionRepresentation::Scaling : state) );
  
  // Depending on state, highlight appropriate parts of representation
//   int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkRectangularBoundingRegionRepresentation::MoveF0:
    case vtkRectangularBoundingRegionRepresentation::MoveF1:
    case vtkRectangularBoundingRegionRepresentation::MoveF2:
    case vtkRectangularBoundingRegionRepresentation::MoveF3:
    case vtkRectangularBoundingRegionRepresentation::MoveF4:
    case vtkRectangularBoundingRegionRepresentation::MoveF5:
      this->HighlightOutline(0);
//       handle = this->HighlightHandle(this->CurrentHandle);
//       this->HighlightFace(handle);
      break;
    case vtkRectangularBoundingRegionRepresentation::Rotating:
      this->HighlightOutline(0);
//       this->HighlightHandle(NULL);
      this->HighlightFace(this->HexPicker->GetCellId());
      break;
    case vtkRectangularBoundingRegionRepresentation::Translating:
    case vtkRectangularBoundingRegionRepresentation::Scaling:
      this->HighlightOutline(1);
//       this->HighlightHandle(this->Handle[6]);
      this->HighlightFace(-1);
      break;
    default:
      this->HighlightOutline(0);
//       this->HighlightHandle(NULL);
      this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkRectangularBoundingRegionRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->InclusionActor->GetBounds());
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
  this->InclusionActor->ReleaseGraphicsResources(w);
//   this->BoundingFaceActor->ReleaseGraphicsResources(w);
  this->HexFace->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();
  
  count += this->InclusionActor->RenderOpaqueGeometry(v);
//   count += this->BoundingFaceActor->RenderOpaqueGeometry(v);
  count += this->HexFace->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();
  
  count += this->InclusionActor->RenderTranslucentPolygonalGeometry(v);
//   count += this->BoundingFaceActor->RenderTranslucentPolygonalGeometry(v);
  count += this->HexFace->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  result |= this->InclusionActor->HasTranslucentPolygonalGeometry();
//   result |= this->BoundingFaceActor->HasTranslucentPolygonalGeometry();

  // If the face is not selected, we are not really rendering translucent faces,
  // hence don't bother taking it's opacity into consideration.
  // Look at BUG #7301.
  if (this->HexFace->GetProperty() == this->SelectedFaceProperty)
    {
    result |= this->HexFace->HasTranslucentPolygonalGeometry();
    }

  return result;
}

//----------------------------------------------------------------------------
// int vtkRectangularBoundingRegionRepresentation::HighlightHandle(vtkProp *prop)
// {
//   // first unhighlight anything picked
//   this->HighlightOutline(0);
//   if ( this->CurrentHandle )
//     {
//     this->CurrentHandle->SetProperty(this->HandleProperty);
//     }
// 
//   this->CurrentHandle = static_cast<vtkActor *>(prop);
// 
//   if ( this->CurrentHandle )
//     {
//     this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
//     for (int i=0; i<6; i++) //find attached face
//       {
//       if ( this->CurrentHandle == this->Handle[i] )
//         {
//         return i;
//         }
//       }
//     }
//   
//   if ( this->CurrentHandle == this->Handle[6] )
//     {
//     this->HighlightOutline(1);
//     return 6;
//     }
//   
//   return -1;
// }

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::HighlightFace(int cellId)
{
  if ( cellId >= 0 )
    {
    vtkIdType npts;
    vtkIdType *pts;
    vtkCellArray *cells = this->HexFacePolyData->GetPolys();
    this->InclusionPolyData->GetCellPoints(cellId, npts, pts);
    this->HexFacePolyData->Modified();
    cells->ReplaceCell(0,npts,pts);
    this->CurrentHexFace = cellId;
    this->HexFace->SetProperty(this->SelectedFaceProperty);
    if ( !this->CurrentHandle )
      {
      this->CurrentHandle = this->HexFace;
      }
    }
  else
    {
    this->HexFace->SetProperty(this->FaceProperty);
    this->CurrentHexFace = -1;
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::HighlightOutline(int highlight)
{
  if ( highlight )
    {
    this->InclusionActor->SetProperty(this->SelectedOutlineProperty);
//     this->BoundingFaceActor->SetProperty(this->SelectedOutlineProperty);
    }
  else
    {
    this->InclusionActor->SetProperty(this->InclusionProperty);
//     this->BoundingFaceActor->SetProperty(this->BoundingFaceProperty);
    }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";

  if ( this->FaceProperty )
    {
    os << indent << "Face Property: " << this->FaceProperty << "\n";
    }
  else
    {
    os << indent << "Face Property: (none)\n";
    }
  if ( this->SelectedFaceProperty )
    {
    os << indent << "Selected Face Property: " 
       << this->SelectedFaceProperty << "\n";
    }
  else
    {
    os << indent << "Selected Face Property: (none)\n";
    }

  if ( this->SelectedOutlineProperty )
    {
    os << indent << "Selected Outline Property: " 
       << this->SelectedOutlineProperty << "\n";
    }
  else
    {
    os << indent << "Selected Outline Property: (none)\n";
    }

}

