/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "vtkBoundingRegion3DRepresentation.h"

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


vtkStandardNewMacro(vtkBoundingRegion3DRepresentation);

enum View {XY=2,YZ=0,XZ=1,VOL=3};

//----------------------------------------------------------------------------
vtkBoundingRegion3DRepresentation::vtkBoundingRegion3DRepresentation()
{
  // The initial state
  this->InteractionState = vtkBoundingRegion3DRepresentation::Outside;
  this->BoundingRegion = NULL;
  memset(this->InclusionOffset, 0, 3*sizeof(double));
  memset(this->ExclusionOffset, 0, 3*sizeof(double));

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Construct the poly data representing the bounding region
  this->VolumePolyData = vtkPolyData::New();
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

  this->VolumeMapper = vtkPolyDataMapper::New();
  this->VolumeMapper->SetInput(this->VolumePolyData);
  this->VolumeMapper->SetLookupTable(this->InclusionLUT);
  this->VolumeActor = vtkActor::New();
  this->VolumeActor->SetMapper(this->VolumeMapper);
  this->VolumeActor->SetProperty(this->InclusionProperty);

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
  this->VolumePicker = vtkCellPicker::New();
  this->VolumePicker->SetTolerance(0.01);
  for (unsigned int i=0; i<6; i++)
    this->VolumePicker->AddPickList(this->MarginActor[i]);
  this->VolumePicker->PickFromListOn();

  this->CurrentHandle = NULL;
}

//----------------------------------------------------------------------------
vtkBoundingRegion3DRepresentation::~vtkBoundingRegion3DRepresentation()
{
  //TODO: Review deletes

  this->VolumeActor->Delete();
  this->VolumeMapper->Delete();
  this->VolumePolyData->Delete();


  this->VolumePicker->Delete();

  this->BoundingBox->Delete();

  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->InclusionProperty->Delete();
  this->SelectedOutlineProperty->Delete();
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::GetPolyData(vtkPolyData *pd)
{
  pd->SetPoints(this->VolumePolyData->GetPoints());
  pd->SetPolys(this->VolumePolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::reset()
{
}


//----------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::StartWidgetInteraction(double e[2])
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
void vtkBoundingRegion3DRepresentation::WidgetInteraction(double e[2])
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
  if ( this->LastPicker == this->VolumePicker )
    {
    this->VolumePicker->GetPickPosition(pos);
    }
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveLeft )
    {
    this->MoveLeftMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveRight )
    {
    this->MoveRightMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveTop )
    {
    this->MoveTopMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveBottom )
    {
    this->MoveBottomMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveUpper )
    {
    this->MoveUpperMargin(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoundingRegion3DRepresentation::MoveLower)
    {
    this->MoveLowerMargin(prevPickPoint,pickPoint);
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

// //----------------------------------------------------------------------------
// void vtkBoundingRegion3DRepresentation::MoveFace(double *p1, double *p2, double *dir,
//                                     double *x1, double *x2, double *x3, double *x4,
//                                     double *x5)
//   {
//   int i;
//   double v[3], v2[3];
// 
//   for (i=0; i<3; i++)
//     {
//     v[i] = p2[i] - p1[i];
//     v2[i] = dir[i];
//     }
// 
//   vtkMath::Normalize(v2);
//   double f = vtkMath::Dot(v,v2);
//   
//   for (i=0; i<3; i++)
//     {
//     v[i] = f*v2[i];
//   
//     x1[i] += v[i];
//     x2[i] += v[i];
//     x3[i] += v[i];
//     x4[i] += v[i];
//     x5[i] += v[i];
//     }
// }
// 
// //----------------------------------------------------------------------------
// void vtkBoundingRegion3DRepresentation::GetDirection(const double Nx[3],const double Ny[3],
//                                         const double Nz[3], double dir[3])
// {
//   double dotNy, dotNz;
//   double y[3];
// 
//   if(vtkMath::Dot(Nx,Nx)!=0)
//     {
//     dir[0] = Nx[0];
//     dir[1] = Nx[1];
//     dir[2] = Nx[2];
//     }
//   else 
//     {
//     dotNy = vtkMath::Dot(Ny,Ny);
//     dotNz = vtkMath::Dot(Nz,Nz);
//     if(dotNy != 0 && dotNz != 0)
//       {
//       vtkMath::Cross(Ny,Nz,dir);
//       }
//     else if(dotNy != 0)
//       {
//       //dir must have been initialized to the 
//       //corresponding coordinate direction before calling
//       //this method
//       vtkMath::Cross(Ny,dir,y);
//       vtkMath::Cross(y,Ny,dir);
//       }
//     else if(dotNz != 0)
//       {
//       //dir must have been initialized to the 
//       //corresponding coordinate direction before calling
//       //this method
//       vtkMath::Cross(Nz,dir,y);
//       vtkMath::Cross(y,Nz,dir);
//       }
//     }
// }

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveLeftMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
// 
//   const int X = 0;
// 
//   assert(ViewType == XY || ViewType == XZ);
//   int contBorder1 = ViewType == XY?TOP:UPPER;
//   int contBorder2 = ViewType == XY?BOTTOM:LOWER;
// 
//   if (ViewType == XY)
//   {
//     pts[3*3 + X] = p2[X]; 
//     pts[3*0 + X] = p2[X]; 
//   }else if (ViewType == XZ)
//   {
//     double shift = (p2[X] - p1[X]);
//     for (int s=0; s < m_numSlices; s++)
//     {
//       pts[3*2*s + X] += shift;
//     }
//   }
// 
//   InclusionOffset[X] += p2[X] - m_lastInclusionMargin[X];
//   m_lastInclusionMargin[X] = p2[X];
//   std::cout << "New Left Inclusion Offset " << InclusionOffset[X] << std::endl;
// 
//   this->MarginPolyData[LEFT]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveRightMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
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
//   std::cout << "Old Right Exclusion Margin " << m_lastExclusionMargin[X] << std::endl;
//   std::cout << "Mouse Position " << p2[X] << std::endl;
//   ExclusionOffset[X] += m_lastExclusionMargin[X] - p2[X];
//   m_lastExclusionMargin[X] = p2[X];
//   std::cout << "New Right Exclusion Offset " << ExclusionOffset[X] << std::endl;
// 
//   this->MarginPolyData[RIGHT]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveTopMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
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
//   InclusionOffset[Y] += p2[Y] - m_lastInclusionMargin[Y];
//   m_lastInclusionMargin[Y] = p2[Y];
//   std::cout << "New Top Inclusion Offset " << InclusionOffset[Y] << std::endl;
// 
//   this->MarginPolyData[TOP]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveBottomMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
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
//   ExclusionOffset[Y] += m_lastExclusionMargin[Y] - p2[Y];
//   m_lastExclusionMargin[Y] = p2[Y];
//   std::cout << "New Botton Exclusion Offset " << ExclusionOffset[Y] << std::endl;
// 
//   this->MarginPolyData[BOTTOM]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}
//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveUpperMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
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
//   InclusionOffset[Z] += p2[Z] - m_lastInclusionMargin[Z];
//   m_lastInclusionMargin[Z] = p2[Z];
//   std::cout << "New Upper Inclusion Offset " << InclusionOffset[Z] << std::endl;
// 
//   this->MarginPolyData[UPPER]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::MoveLowerMargin(double* p1, double* p2)
{
//   double *pts =
//     static_cast<vtkDoubleArray *>(this->MarginPoints->GetData())->GetPointer(0);
// 
//   const int Z = 2;
// 
//   assert(ViewType == YZ || ViewType == XZ);
//   int contBorder1 = ViewType == YZ?TOP:LEFT;
//   int contBorder2 = ViewType == YZ?BOTTOM:RIGHT;
// 
//   int lastSlice = m_numSlices - 1;
//   int lastMarginPoint = lastSlice*2;
//   pts[3*(lastMarginPoint) + Z] = p2[Z];
//   pts[3*(lastMarginPoint+1) + Z] = p2[Z];
// 
//   ExclusionOffset[Z] += m_lastExclusionMargin[Z] - p2[Z];
//   m_lastExclusionMargin[Z] = p2[Z];
//   std::cout << "New Lower Exclusion Offset " << ExclusionOffset[Z] << std::endl;
// 
//   this->MarginPolyData[LOWER]->Modified();
//   this->MarginPolyData[contBorder1]->Modified();
//   this->MarginPolyData[contBorder2]->Modified();
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::CreateDefaultProperties()
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
void vtkBoundingRegion3DRepresentation::CreateVolume()
{
//   std::cout << "Created XY FACE" << std::endl;
  //BoundingRegion->UpdateWholeExtent();

  m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Margin
  m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Margin
  m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Margin
  m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Margin

  double point[3];
  vtkCellArray *inLines;
  vtkSmartPointer<vtkIntArray> lineData;
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

  // We need it to get lower limits
  int numPoints = this->BoundingRegion->GetPoints()->GetNumberOfPoints();
  this->BoundingRegion->GetPoint(numPoints-1,point);
  m_lastExclusionMargin[2] = point[2];

  this->MarginPoints->SetNumberOfPoints(4);

  /// Top Inclusion Margin (0 - 1)
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);

  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  // Point 0
  BoundingRegion->GetPoint(0*4+0,point);
  point[2] = -0.1;
  this->MarginPoints->SetPoint(0, point);

  //Point 1
  BoundingRegion->GetPoint(0*4+1,point);
  point[2] = -0.1;
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
  BoundingRegion->GetPoint(0*4+2,point);
  point[2] = -0.1;
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
  BoundingRegion->GetPoint(0*4+3,point);
  point[2] = -0.1;
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
void vtkBoundingRegion3DRepresentation::SetBoundingRegion(vtkSmartPointer<vtkPolyData> region)
{
  BoundingRegion = region;
  BoundingRegion->Update();
  VolumePolyData->SetPoints(BoundingRegion->GetPoints());
  VolumePolyData->SetPolys(BoundingRegion->GetPolys());
  VolumePolyData->GetCellData()->SetScalars(BoundingRegion->GetCellData()->GetScalars("Type"));
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::PlaceWidget(double bds[6])
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

  this->ValidPick = 1; //since we have set up widget
}

//----------------------------------------------------------------------------
int vtkBoundingRegion3DRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkBoundingRegion3DRepresentation::Outside;
    return this->InteractionState;
    }
  
  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = NULL;
  this->CurrentHandle = NULL;
  this->VolumePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->VolumePicker->GetPath();
  if ( path != NULL )
  {
    this->LastPicker = this->VolumePicker;
    this->ValidPick = 1;
    
    this->CurrentHandle = 
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (this->CurrentHandle == this->MarginActor[LEFT])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveLeft;
    }
    else if (this->CurrentHandle == this->MarginActor[RIGHT])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveRight;
    } 
    else if (this->CurrentHandle == this->MarginActor[TOP])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveTop;
    } 
    else if (this->CurrentHandle == this->MarginActor[BOTTOM])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveBottom;
    }
    else if (this->CurrentHandle == this->MarginActor[UPPER])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveUpper;
    } 
    else if (this->CurrentHandle == this->MarginActor[LOWER])
    {
      this->InteractionState = vtkBoundingRegion3DRepresentation::MoveLower;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    this->InteractionState = vtkBoundingRegion3DRepresentation::Outside;
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = ( state < vtkBoundingRegion3DRepresentation::Outside ? vtkBoundingRegion3DRepresentation::Outside :state);
  
  // Depending on state, highlight appropriate parts of representation
//   int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkBoundingRegion3DRepresentation::MoveLeft:
    case vtkBoundingRegion3DRepresentation::MoveRight:
    case vtkBoundingRegion3DRepresentation::MoveTop:
    case vtkBoundingRegion3DRepresentation::MoveBottom:
    case vtkBoundingRegion3DRepresentation::MoveUpper:
    case vtkBoundingRegion3DRepresentation::MoveLower:
      this->HighlightMargin(this->CurrentHandle);
      break;
    default:
//       this->HighlightOutline(0);
      this->HighlightMargin(NULL);
//       this->HighlightHandle(NULL);
//       this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkBoundingRegion3DRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->VolumeActor->GetBounds());
  return this->BoundingBox->GetBounds();
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::BuildRepresentation()
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
void vtkBoundingRegion3DRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->VolumeActor->ReleaseGraphicsResources(w);
  for (unsigned int i=0; i<6; i++)
    this->MarginActor[i]->ReleaseGraphicsResources(w);
//   this->BoundingFaceActor->ReleaseGraphicsResources(w);
//   this->HexFace->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkBoundingRegion3DRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  count += this->VolumeActor->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkBoundingRegion3DRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  count += this->VolumeActor->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkBoundingRegion3DRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  result |= this->VolumeActor->HasTranslucentPolygonalGeometry();
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
// int vtkBoundingRegion3DRepresentation::HighlightHandle(vtkProp *prop)
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
void vtkBoundingRegion3DRepresentation::HighlightMargin(vtkActor* actor)
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
// void vtkBoundingRegion3DRepresentation::HighlightFace(int cellId)
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
void vtkBoundingRegion3DRepresentation::HighlightOutline(int highlight)
{
  if ( highlight )
    {
    this->VolumeActor->SetProperty(this->SelectedOutlineProperty);
    this->MarginActor[TOP]->SetProperty(this->SelectedOutlineProperty);
//     this->BoundingFaceActor->SetProperty(this->SelectedOutlineProperty);
    }
  else
    {
    this->VolumeActor->SetProperty(this->InclusionProperty);
    this->MarginActor[TOP]->SetProperty(this->InclusionProperty);
//     this->BoundingFaceActor->SetProperty(this->BoundingFaceProperty);
    }
}

//----------------------------------------------------------------------------
void vtkBoundingRegion3DRepresentation::PrintSelf(ostream& os, vtkIndent indent)
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

