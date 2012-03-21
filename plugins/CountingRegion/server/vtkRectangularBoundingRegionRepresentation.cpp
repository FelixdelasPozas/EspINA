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

enum View {XY=2,YZ=0,XZ=1,VOL=3};

//----------------------------------------------------------------------------
vtkRectangularBoundingRegionRepresentation::vtkRectangularBoundingRegionRepresentation()
{
  // The initial state
  this->InteractionState = vtkRectangularBoundingRegionRepresentation::Outside;
  this->ViewType = VOL; //Default 3D View
  this->Slice = 0;
  this->Region = NULL;
  memset(this->InclusionOffset, 0, 3*sizeof(double));
  memset(this->ExclusionOffset, 0, 3*sizeof(double));

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Construct the poly data representing the bounding region
  this->RegionPolyData = vtkPolyData::New();
  this->InclusionLUT = vtkLookupTable::New();
  this->InclusionLUT->SetNumberOfTableValues(2);
  this->InclusionLUT->Build();
  InclusionLUT->SetTableValue(0,1,0,0);
  InclusionLUT->SetTableValue(1,0,1,0);

  this->MarginPoints   = vtkPoints::New(VTK_DOUBLE);
  this->MarginPoints->SetNumberOfPoints(4);//line sides;
  for (unsigned int i=0; i<6; i++)
  {
    // Construct the poly data representing the margins
    this->MarginPolyData[i] = vtkPolyData::New();
    this->MarginMapper[i]   = vtkPolyDataMapper::New();
    this->MarginActor[i]    = vtkActor::New();

    this->MarginPolyData[i]->SetPoints(this->MarginPoints);
    this->MarginMapper[i]->SetInput(this->MarginPolyData[i]);
    this->MarginMapper[i]->SetLookupTable(this->InclusionLUT);
    this->MarginActor[i]->SetMapper(this->MarginMapper[i]);
    this->MarginActor[i]->SetProperty(this->InclusionProperty);
  }

  this->RegionMapper = vtkPolyDataMapper::New();
  this->RegionMapper->SetInput(this->RegionPolyData);
  this->RegionMapper->SetLookupTable(this->InclusionLUT);
  this->RegionActor = vtkActor::New();
  this->RegionActor->SetMapper(this->RegionMapper);
  this->RegionActor->SetProperty(this->InclusionProperty);

  // Define the point coordinates
  double bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;
  this->BoundingBox = vtkBox::New();
  this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->RegionPicker = vtkCellPicker::New();
  this->RegionPicker->SetTolerance(0.01);
  for (unsigned int i=0; i<6; i++)
    this->RegionPicker->AddPickList(this->MarginActor[i]);
  this->RegionPicker->PickFromListOn();
  
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
  //TODO: Review deletes

  this->RegionActor->Delete();
  this->RegionMapper->Delete();
  this->RegionPolyData->Delete();


  this->RegionPicker->Delete();

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
  pd->SetPoints(this->RegionPolyData->GetPoints());
  pd->SetPolys(this->RegionPolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::reset()
{
  if (ViewType == XY)
    CreateXYFace();
  else if (ViewType == YZ)
    CreateYZFace();
  else if (ViewType == XZ)
    CreateXZFace();
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
  if ( this->LastPicker == this->RegionPicker )
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
  if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveLeft )
    {
    this->MoveLeftMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveRight )
    {
    this->MoveRightMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveTop )
    {
    this->MoveTopMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveBottom )
    {
    this->MoveBottomMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveUpper )
    {
    this->MoveUpperMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkRectangularBoundingRegionRepresentation::MoveLower)
    {
    this->MoveLowerMargin(prevPickPoint,pickPoint);
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
void vtkRectangularBoundingRegionRepresentation::MoveLeftMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int X = 0;

  assert(ViewType == XY || ViewType == XZ);
  int contBorder1 = ViewType == XY?TOP:UPPER;
  int contBorder2 = ViewType == XY?BOTTOM:LOWER;

  if (ViewType == XY)
  {
    pts[3*3 + X] = p2[X]; 
    pts[3*0 + X] = p2[X]; 
  }else if (ViewType == XZ)
  {
    double shift = (p2[X] - p1[X]);
    for (int s=0; s < m_numSlices; s++)
    {
      pts[3*2*s + X] += shift;
    }
  }

  InclusionOffset[X] += p2[X] - m_lastInclusionMargin[X];
  m_lastInclusionMargin[X] = p2[X];
  std::cout << "New Left Inclusion Offset " << InclusionOffset[X] << std::endl;

  this->MarginPolyData[LEFT]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveRightMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int X = 0;

  assert(ViewType == XY || ViewType == XZ);
  int contBorder1 = ViewType == XY?TOP:UPPER;
  int contBorder2 = ViewType == XY?BOTTOM:LOWER;
  
  if (ViewType == XY)
  {
    pts[3*1 + X] = p2[X];
    pts[3*2 + X] = p2[X];
  }else if (ViewType == XZ)
  {
    double shift = (p2[X] - p1[X]);
    for (int s=0; s < m_numSlices; s++)
    {
      pts[3*(2*s+1) + X] += shift;
    }
  }

  std::cout << "Old Right Exclusion Offset " << ExclusionOffset[X] << std::endl;
  std::cout << "Old Right Exclusion Margin " << m_lastExclusionMargin[X] << std::endl;
  std::cout << "Mouse Position " << p2[X] << std::endl;
  ExclusionOffset[X] += m_lastExclusionMargin[X] - p2[X];
  m_lastExclusionMargin[X] = p2[X];
  std::cout << "New Right Exclusion Offset " << ExclusionOffset[X] << std::endl;

  this->MarginPolyData[RIGHT]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveTopMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int Y = 1;

  assert(ViewType == XY || ViewType == YZ);
  int contBorder1 = ViewType == XY?LEFT:UPPER;
  int contBorder2 = ViewType == XY?RIGHT:LOWER;

  if (ViewType == XY)
  {
    pts[3*0 + Y] = p2[Y]; 
    pts[3*1 + Y] = p2[Y]; 
  }else
  {
    double shift = (p2[Y] - p1[Y]);
    for (int s=0; s < m_numSlices; s++)
    {
      pts[3*(2*s+1) + Y] += shift;
    }
  }

  InclusionOffset[Y] += p2[Y] - m_lastInclusionMargin[Y];
  m_lastInclusionMargin[Y] = p2[Y];
  std::cout << "New Top Inclusion Offset " << InclusionOffset[Y] << std::endl;

  this->MarginPolyData[TOP]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveBottomMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int Y = 1;

  assert(ViewType == XY || ViewType == YZ);
  int contBorder1 = ViewType == XY?LEFT:UPPER;
  int contBorder2 = ViewType == XY?RIGHT:LOWER;

  if (ViewType == XY)
  {
    pts[3*2 + Y] = p2[Y]; 
    pts[3*3 + Y] = p2[Y]; 
  }else if (ViewType == YZ)
  {
    double shift = (p2[Y] - p1[Y]);
    for (int s=0; s < m_numSlices; s++)
    {
      pts[3*2*s + Y] += shift;
    }
  }else
    assert(false);

  ExclusionOffset[Y] += m_lastExclusionMargin[Y] - p2[Y];
  m_lastExclusionMargin[Y] = p2[Y];
  std::cout << "New Botton Exclusion Offset " << ExclusionOffset[Y] << std::endl;

  this->MarginPolyData[BOTTOM]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveUpperMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int Z = 2;

  assert(ViewType == YZ || ViewType == XZ);
  int contBorder1 = ViewType == YZ?TOP:LEFT;
  int contBorder2 = ViewType == YZ?BOTTOM:RIGHT;

  pts[3*0 + Z] = p2[Z]; 
  pts[3*1 + Z] = p2[Z]; 

  InclusionOffset[Z] += p2[Z] - m_lastInclusionMargin[Z];
  m_lastInclusionMargin[Z] = p2[Z];
  std::cout << "New Upper Inclusion Offset " << InclusionOffset[Z] << std::endl;

  this->MarginPolyData[UPPER]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::MoveLowerMargin(double* p1, double* p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  const int Z = 2;

  assert(ViewType == YZ || ViewType == XZ);
  int contBorder1 = ViewType == YZ?TOP:LEFT;
  int contBorder2 = ViewType == YZ?BOTTOM:RIGHT;

  int lastSlice = m_numSlices - 1;
  int lastMarginPoint = lastSlice*2;
  pts[3*(lastMarginPoint) + Z] = p2[Z];
  pts[3*(lastMarginPoint+1) + Z] = p2[Z];

  ExclusionOffset[Z] += m_lastExclusionMargin[Z] - p2[Z];
  m_lastExclusionMargin[Z] = p2[Z];
  std::cout << "New Lower Exclusion Offset " << ExclusionOffset[Z] << std::endl;

  this->MarginPolyData[LOWER]->Modified();
  this->MarginPolyData[contBorder1]->Modified();
  this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
// Loop through all points and translate them
void vtkRectangularBoundingRegionRepresentation::Translate(double *p1, double *p2)
{
//   assert(false);
  /*
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
    */
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::Scale(double *vtkNotUsed(p1),
                                 double *vtkNotUsed(p2),
                                 int vtkNotUsed(X),
                                 int Y)
{
//   assert(false);
  /*
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
    */
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::ComputeNormals()
{
//   assert(false);
  /*
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
    */
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
//   std::cout << "Created XY FACE" << std::endl;
  Region->UpdateWholeExtent();

  m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Margin
  m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Margin
  m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Margin
  m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Margin
  
  double point[3];
  vtkCellArray *inLines;
  vtkSmartPointer<vtkIntArray> lineData;
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

  // We need it to get lower limits
  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();
  this->Region->GetOutput()->GetPoint(numPoints-1,point);
  m_lastExclusionMargin[2] = point[2];

  this->MarginPoints->SetNumberOfPoints(4);

  /// Top Inclusion Margin (0 - 1)
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);

  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  // Point 0
  Region->GetOutput()->GetPoint(0*4+0,point);
  this->MarginPoints->SetPoint(0, point);

  //Point 1
  Region->GetOutput()->GetPoint(0*4+1,point);
  this->MarginPoints->SetPoint(1, point);
  m_lastInclusionMargin[1] = point[1];
  m_lastInclusionMargin[2] = point[2];

  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);

  this->MarginPolyData[TOP]->SetLines(inLines);
  this->MarginPolyData[TOP]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[TOP]->Modified();

  /// Right Inclusion Margin (1 - 2)
  line->GetPointIds()->SetId(0,1);
  line->GetPointIds()->SetId(1,2);

  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  // Point 2
  Region->GetOutput()->GetPoint(0*4+2,point);
  this->MarginPoints->SetPoint(2, point);
  m_lastExclusionMargin[0] = point[0];

  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);

  this->MarginPolyData[RIGHT]->SetLines(inLines);
  this->MarginPolyData[RIGHT]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[RIGHT]->Modified();

  /// Left Inclusion Margin (3 - 0)
  line->GetPointIds()->SetId(0,3);
  line->GetPointIds()->SetId(1,0);

  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  // Point 3
  Region->GetOutput()->GetPoint(0*4+3,point);
  this->MarginPoints->SetPoint(3, point);
  m_lastInclusionMargin[0] = point[0];
  m_lastExclusionMargin[1] = point[1];

  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);

  this->MarginPolyData[LEFT]->SetLines(inLines);
  this->MarginPolyData[LEFT]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[LEFT]->Modified();

  /// Bottom Inclusion Margin (2 - 3)
  line->GetPointIds()->SetId(0,2);
  line->GetPointIds()->SetId(1,3);
  
  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);

  this->MarginPolyData[BOTTOM]->SetLines(inLines);
  this->MarginPolyData[BOTTOM]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[BOTTOM]->Modified();

  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateYZFace()
{
//   std::cout << "Created YZ FACE" << std::endl;
  this->Region->UpdateWholeExtent();
  
  m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Margin
  m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Margin
  m_prevInclusion[2] = InclusionOffset[2]; // Use in Move Upper Margin
  m_prevExclusion[2] = ExclusionOffset[2]; // Use in Move Lower Margin
  
  int numPoints = this->Region->GetOutput()->GetPoints()->GetNumberOfPoints();  
  unsigned int numSlices = numPoints/4;
  int numLateralMargins = numSlices - 1;
  m_numSlices = numSlices;
  
  double point[3];
  vtkCellArray *inLines;
  vtkSmartPointer<vtkIntArray> lineData;
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

  // We need it to get right limits
  this->Region->GetOutput()->GetPoint(2,point);
  m_lastExclusionMargin[0] = point[0];
  for (unsigned int s=1; s < numSlices; s++)
  {
    this->Region->GetOutput()->GetPoint(s*4+2,point);
    m_lastExclusionMargin[0] = std::max(m_lastExclusionMargin[0], point[0]);
  }
  
  this->MarginPoints->SetNumberOfPoints(numSlices*2);
  
  /// Upper Margin (0 - 1) // Point index refer to MarginPoint indices
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);
  
  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();
  
  // Point 0
  this->Region->GetOutput()->GetPoint(0*4+3,point);
  this->MarginPoints->SetPoint(0, point);
  
  // Point 1
  this->Region->GetOutput()->GetPoint(0*4+0,point);
  this->MarginPoints->SetPoint(1, point);
  m_lastInclusionMargin[0] = point[0];
  m_lastInclusionMargin[2] = point[2];
  
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
    
  this->MarginPolyData[UPPER]->SetLines(inLines);
  this->MarginPolyData[UPPER]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[UPPER]->Modified();
  
  /// Loop over slices and create Top/Bottom Margins
  vtkCellArray *topLines = vtkCellArray::New();
  topLines->Allocate(topLines->EstimateSize(numLateralMargins,2));
  vtkSmartPointer<vtkIntArray> topLineData = vtkSmartPointer<vtkIntArray>::New();
  topLineData->Allocate(numLateralMargins);
  
  vtkCellArray *bottomLines = vtkCellArray::New();
  bottomLines->Allocate(bottomLines->EstimateSize(numLateralMargins,2));
  vtkSmartPointer<vtkIntArray> bottomLineData = vtkSmartPointer<vtkIntArray>::New();
  bottomLineData->Allocate(numLateralMargins);
  
  for (unsigned int slice=1; slice< numSlices; slice++)
  {
    int prevSlice = slice - 1;
    
    // Add new points
    // Point Slice_0
    Region->GetOutput()->GetPoint(slice*4+3,point);
    this->MarginPoints->SetPoint(2*slice, point);//BOTTOM
    m_lastExclusionMargin[1] = point[1];
    
    // Point Slice_1
    Region->GetOutput()->GetPoint(slice*4+0,point);
    this->MarginPoints->SetPoint(2*slice+1, point);//TOP
    m_lastInclusionMargin[1] = point[1];
    m_lastExclusionMargin[2] = point[2];
    
    /// Bottom Margin (prevSlice_0 - Slice_0)
    line->GetPointIds()->SetId(0,2*prevSlice);
    line->GetPointIds()->SetId(1,2*slice);
    bottomLines->InsertNextCell(line);
    bottomLineData->InsertNextValue(0);
    
    /// Top Margin (prevSlice_1 - Slice_1)
    line->GetPointIds()->SetId(0,2*prevSlice+1);
    line->GetPointIds()->SetId(1,2*slice+1);
    topLines->InsertNextCell(line);
    topLineData->InsertNextValue(255);
    
  }
  this->MarginPolyData[TOP]->SetLines(topLines);
  this->MarginPolyData[TOP]->GetCellData()->SetScalars(topLineData);
  this->MarginPolyData[TOP]->Modified();
  
  this->MarginPolyData[BOTTOM]->SetLines(bottomLines);
  this->MarginPolyData[BOTTOM]->GetCellData()->SetScalars(bottomLineData);
  this->MarginPolyData[BOTTOM]->Modified();
  
  
  // Lower Margin
  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();
  line->GetPointIds()->SetId(0,2*(numSlices-1));
  line->GetPointIds()->SetId(1,2*(numSlices-1)+1);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
    
  this->MarginPolyData[LOWER]->SetLines(inLines);
  this->MarginPolyData[LOWER]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[LOWER]->Modified();
  
  BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::CreateXZFace()
{
//   std::cout << "Created XZ FACE" << std::endl;
  Region->UpdateWholeExtent();

  m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Margin
  m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Margin
  m_prevInclusion[2] = InclusionOffset[2]; // Use in Move Upper Margin
  m_prevExclusion[2] = ExclusionOffset[2]; // Use in Move Lower Margin

  int numPoints = Region->GetOutput()->GetPoints()->GetNumberOfPoints();  
  unsigned int numSlices = numPoints/4;
  int numLateralMargins = numSlices - 1;
  m_numSlices = numSlices;

  double point[3];
  vtkCellArray *inLines;
  vtkSmartPointer<vtkIntArray> lineData;
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

  // We need it to get right limits
  this->Region->GetOutput()->GetPoint(0,point);
  m_lastInclusionMargin[1] = point[1];
  this->Region->GetOutput()->GetPoint(3,point);
  m_lastExclusionMargin[1] = point[1];
  for (unsigned int s=1; s < numSlices; s++)
  {
    this->Region->GetOutput()->GetPoint(s*4+0,point);
    m_lastInclusionMargin[1] = std::min(m_lastInclusionMargin[1], point[1]);
    this->Region->GetOutput()->GetPoint(s*4+3,point);
    m_lastExclusionMargin[1] = std::max(m_lastExclusionMargin[1], point[1]);
  }
  
  this->MarginPoints->SetNumberOfPoints(numSlices*2);
  
  /// Upper Margin (0 - 1)
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);
  
  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();
  
  // Point 0
  Region->GetOutput()->GetPoint(0*4+0,point);
  this->MarginPoints->SetPoint(0, point);
  
  // Point 1
  Region->GetOutput()->GetPoint(0*4+1,point);
  this->MarginPoints->SetPoint(1, point);
  m_lastInclusionMargin[2] = point[2];
  
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(255);
    
  this->MarginPolyData[UPPER]->SetLines(inLines);
  this->MarginPolyData[UPPER]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[UPPER]->Modified();
  
  /// Loop over slices and create Top/Bottom Margins
  vtkCellArray *leftLines = vtkCellArray::New();
  leftLines->Allocate(leftLines->EstimateSize(numLateralMargins,2));
  vtkSmartPointer<vtkIntArray> leftLineData = vtkSmartPointer<vtkIntArray>::New();
  leftLineData->Allocate(numLateralMargins);
  
  vtkCellArray *rightLines = vtkCellArray::New();
  rightLines->Allocate(rightLines->EstimateSize(numLateralMargins,2));
  vtkSmartPointer<vtkIntArray> rightLineData = vtkSmartPointer<vtkIntArray>::New();
  rightLineData->Allocate(numLateralMargins);
  
  for (unsigned int slice=1; slice< numSlices; slice++)
  {
    int prevSlice = slice - 1;
	
    // Add new points
    // Point Slice_0
    Region->GetOutput()->GetPoint(slice*4+0,point);
    this->MarginPoints->SetPoint(2*slice+0, point);//TOP
    m_lastInclusionMargin[0] = point[0];
    // Point Slice_1
    Region->GetOutput()->GetPoint(slice*4+1,point);
    this->MarginPoints->SetPoint(2*slice+1, point);//RIGHT
    m_lastExclusionMargin[0] = point[0];
    m_lastExclusionMargin[2] = point[2];
    
    /// Left Margin (prevSlice_0 - Slice_0)
    line->GetPointIds()->SetId(0,2*prevSlice+0);
    line->GetPointIds()->SetId(1,2*slice+0);
    leftLines->InsertNextCell(line);
    leftLineData->InsertNextValue(255);
    
    /// Right Margin (prevSlice_1 - Slice_1)
    line->GetPointIds()->SetId(0,2*prevSlice+1);
    line->GetPointIds()->SetId(1,2*slice+1);
    rightLines->InsertNextCell(line);
    rightLineData->InsertNextValue(0);
    
  }
  this->MarginPolyData[LEFT]->SetLines(leftLines);
  this->MarginPolyData[LEFT]->GetCellData()->SetScalars(leftLineData);
  this->MarginPolyData[LEFT]->Modified();
  
  this->MarginPolyData[RIGHT]->SetLines(rightLines);
  this->MarginPolyData[RIGHT]->GetCellData()->SetScalars(rightLineData);
  this->MarginPolyData[RIGHT]->Modified();
  
  
  // Lower Margin
  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();
  line->GetPointIds()->SetId(0,2*(numSlices-1));
  line->GetPointIds()->SetId(1,2*(numSlices-1)+1);
  inLines->InsertNextCell(line);
  lineData->InsertNextValue(0);
    
  this->MarginPolyData[LOWER]->SetLines(inLines);
  this->MarginPolyData[LOWER]->GetCellData()->SetScalars(lineData);
  this->MarginPolyData[LOWER]->Modified();
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
  }
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetSlice(int slice, double spacing[3])
{
  Slice = slice;
//   return;
  
  if (ViewType == VOL)
    return;
  
  int normalDir = (ViewType+2)%3;
  int slicePosition = slice*spacing[normalDir];
  bool showRegion = m_lastInclusionMargin[normalDir] <= slicePosition &&
		    slicePosition <= m_lastExclusionMargin[normalDir];
//   std::cout << "Exclusion on " << normalDir << ": " << m_prevExclusionCoord[normalDir] << std::endl;
  if (showRegion)
  {
    for (int i=0; i<6; i++)
      MarginActor[i]->SetProperty(InclusionProperty);
  }else
  {
    for (int i=0; i<6; i++)
      MarginActor[i]->SetProperty(InvisibleProperty);
    return;
  }
  
  double *pts =
    static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);

  if (ViewType == XY)
  {
    double point[3];
    for (int p=0; p<this->MarginPoints->GetNumberOfPoints(); p++)
    {
      int numFaces = Region->GetOutput()->GetPoints()->GetNumberOfPoints()/4;
      int validSlice = numFaces==2?0:slice;
      
      Region->GetOutput()->GetPoint(validSlice*4+p,point);
      
//       pts[3*p+0] = (p==3||p==0)?Inclusion[0]:point[0];
      pts[3*p+0] = point[0];
      pts[3*p+1] = point[1];
      pts[3*p+2] = point[2];
// 	pts[3*p] = slice; //BUG: get real spacing
    }
  }else
  {
    int normalDirection;
    if (ViewType == YZ)
      normalDirection = 0;
    else
      normalDirection = 1;
    for (int p=0; p<this->MarginPoints->GetNumberOfPoints(); p++)
    {
      pts[3*p+normalDirection] = slice*spacing[normalDirection];
    }
  }
  for(unsigned int i=0; i<6; i++)
    this->MarginPolyData[i]->Modified();
//   std::cout << "View " << ViewType <<  " : " << BoundingFacePoints->GetNumberOfPoints() << std::endl;
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetRegion(vtkPolyDataAlgorithm *region)
{
  Region = region;
  region->Update();
  RegionPolyData->SetPoints(region->GetOutput()->GetPoints());
  RegionPolyData->SetPolys(region->GetOutput()->GetPolys());
  RegionPolyData->GetCellData()->SetScalars(region->GetOutput()->GetCellData()->GetScalars("Type"));
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
  
  this->MarginPoints->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  this->MarginPoints->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  this->MarginPoints->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  this->MarginPoints->SetPoint(3, bounds[0], bounds[3], bounds[4]);

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
  /*
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
  
  */
}

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::SetTransform(vtkTransform* t)
{
//   std::cout << "SetTransform" << std::endl;
  /*
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
*/
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
  this->CurrentHandle = NULL;
  this->RegionPicker->Pick(X,Y,0.0,this->Renderer);
  path = this->RegionPicker->GetPath();
  if ( path != NULL )
  {
    this->LastPicker = this->RegionPicker;
    this->ValidPick = 1;
    
    this->CurrentHandle = 
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (this->CurrentHandle == this->MarginActor[LEFT])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveLeft;
    }
    else if (this->CurrentHandle == this->MarginActor[RIGHT])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveRight;
    } 
    else if (this->CurrentHandle == this->MarginActor[TOP])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveTop;
    } 
    else if (this->CurrentHandle == this->MarginActor[BOTTOM])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveBottom;
    }
    else if (this->CurrentHandle == this->MarginActor[UPPER])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveUpper;
    } 
    else if (this->CurrentHandle == this->MarginActor[LOWER])
    {
      this->InteractionState = vtkRectangularBoundingRegionRepresentation::MoveLower;
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
  state = ( state < vtkRectangularBoundingRegionRepresentation::Outside ? vtkRectangularBoundingRegionRepresentation::Outside : 
            (state > vtkRectangularBoundingRegionRepresentation::Scaling ? vtkRectangularBoundingRegionRepresentation::Scaling : state) );
  
  // Depending on state, highlight appropriate parts of representation
//   int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkRectangularBoundingRegionRepresentation::MoveLeft:
    case vtkRectangularBoundingRegionRepresentation::MoveRight:
    case vtkRectangularBoundingRegionRepresentation::MoveTop:
    case vtkRectangularBoundingRegionRepresentation::MoveBottom:
    case vtkRectangularBoundingRegionRepresentation::MoveUpper:
    case vtkRectangularBoundingRegionRepresentation::MoveLower:
      this->HighlightMargin(this->CurrentHandle);
      break;
    case vtkRectangularBoundingRegionRepresentation::Translating:
    case vtkRectangularBoundingRegionRepresentation::Scaling:
      this->HighlightOutline(1);
//       this->HighlightHandle(this->Handle[6]);
//       this->HighlightFace(-1);
      break;
    default:
//       this->HighlightOutline(0);
      this->HighlightMargin(NULL);
//       this->HighlightHandle(NULL);
//       this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkRectangularBoundingRegionRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->RegionActor->GetBounds());
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
  this->RegionActor->ReleaseGraphicsResources(w);
  for (unsigned int i=0; i<6; i++)
    this->MarginActor[i]->ReleaseGraphicsResources(w);
//   this->BoundingFaceActor->ReleaseGraphicsResources(w);
//   this->HexFace->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();
  
  if (ViewType != VOL)
    for (unsigned int i=0; i<6; i++)
      count += this->MarginActor[i]->RenderOpaqueGeometry(v);
  else
    count += this->RegionActor->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();
  
  if (ViewType != VOL)
    for (unsigned int i=0; i<6; i++)
      count += this->MarginActor[i]->RenderTranslucentPolygonalGeometry(v);
  else
    count += this->RegionActor->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkRectangularBoundingRegionRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  result |= this->RegionActor->HasTranslucentPolygonalGeometry();
  result |= this->MarginActor[TOP]->HasTranslucentPolygonalGeometry();

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
void vtkRectangularBoundingRegionRepresentation::HighlightMargin(vtkActor* actor)
{
  for(unsigned char margin=0; margin < 6; margin++)
  {
    if (this->MarginActor[margin] == actor)
      this->MarginActor[margin]->SetProperty(this->SelectedOutlineProperty);
    else
      this->MarginActor[margin]->SetProperty(this->InclusionProperty);
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
//     if ( !this->CurrentHandle )
//       {
//       this->CurrentHandle = this->HexFace;
//       }
//     }
//   else
//     {
//     this->HexFace->SetProperty(this->FaceProperty);
//     this->CurrentHexFace = -1;
//     }
// }

//----------------------------------------------------------------------------
void vtkRectangularBoundingRegionRepresentation::HighlightOutline(int highlight)
{
  if ( highlight )
    {
    this->RegionActor->SetProperty(this->SelectedOutlineProperty);
    this->MarginActor[TOP]->SetProperty(this->SelectedOutlineProperty);
//     this->BoundingFaceActor->SetProperty(this->SelectedOutlineProperty);
    }
  else
    {
    this->RegionActor->SetProperty(this->InclusionProperty);
    this->MarginActor[TOP]->SetProperty(this->InclusionProperty);
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

