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

#include "vtkCountingFrame3DRepresentation.h"

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


vtkStandardNewMacro(vtkCountingFrame3DRepresentation);

//----------------------------------------------------------------------------
vtkCountingFrame3DRepresentation::vtkCountingFrame3DRepresentation()
: VolumePoints           {nullptr}
, CurrentHexFace         {-1}
, LastPicker             {nullptr}
, FaceProperty           {nullptr}
, SelectedFaceProperty   {nullptr}
, InclusionProperty      {nullptr}
, InvisibleProperty      {nullptr}
, SelectedOutlineProperty{nullptr}
{
  // The initial state
  this->InteractionState = vtkCountingFrame3DRepresentation::Outside;
  this->CountingFrame = nullptr;
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
    this->MarginMapper[i]->SetInputData(this->MarginPolyData[i]);
    this->MarginMapper[i]->SetLookupTable(this->InclusionLUT);
    this->MarginActor[i]->SetMapper(this->MarginMapper[i]);
    this->MarginActor[i]->SetProperty(this->InclusionProperty);
  }

  this->VolumeMapper = vtkPolyDataMapper::New();
  this->VolumeMapper->SetInputData(this->VolumePolyData);
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

  this->CurrentHandle = nullptr;
}

//----------------------------------------------------------------------------
vtkCountingFrame3DRepresentation::~vtkCountingFrame3DRepresentation()
{
  this->VolumeActor->Delete();
  this->VolumeMapper->Delete();
  this->VolumePolyData->Delete();

  this->MarginPoints->Delete();
  for(int i = 0; i < 6; ++i)
  {
    this->MarginActor[i]->Delete();
    this->MarginMapper[i]->Delete();
    this->MarginPolyData[i]->Delete();
  }

  this->VolumePicker->Delete();

  this->BoundingBox->Delete();

  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->InclusionProperty->Delete();
  this->InvisibleProperty->Delete();
  this->SelectedOutlineProperty->Delete();
  this->InclusionLUT->Delete();
}

//----------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::GetPolyData(vtkPolyData *pd)
{
  pd->SetPoints(this->VolumePolyData->GetPoints());
  pd->SetPolys(this->VolumePolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::reset()
{
}

//----------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::StartWidgetInteraction(double e[2])
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
void vtkCountingFrame3DRepresentation::WidgetInteraction(double e[2])
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

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::CreateDefaultProperties()
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
void vtkCountingFrame3DRepresentation::CreateVolume()
{
//   std::cout << "Created XY FACE" << std::endl;
  //CountingFrame->UpdateWholeExtent();

  m_prevInclusion[0] = InclusionOffset[0]; // Use in Move Left Margin
  m_prevExclusion[0] = ExclusionOffset[0]; // Use in Move Right Margin
  m_prevInclusion[1] = InclusionOffset[1]; // Use in Move Top Margin
  m_prevExclusion[1] = ExclusionOffset[1]; // Use in Move Bottom Margin

  double point[3];
  vtkCellArray *inLines;
  vtkSmartPointer<vtkIntArray> lineData;
  vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();

  // We need it to get lower limits
  int numPoints = this->CountingFrame->GetPoints()->GetNumberOfPoints();
  this->CountingFrame->GetPoint(numPoints-1,point);
  m_lastExclusionMargin[2] = point[2];

  this->MarginPoints->SetNumberOfPoints(4);

  /// Top Inclusion Margin (0 - 1)
  line->GetPointIds()->SetId(0,0);
  line->GetPointIds()->SetId(1,1);

  inLines = vtkCellArray::New();
  inLines->Allocate(inLines->EstimateSize(1,2));
  lineData = vtkSmartPointer<vtkIntArray>::New();

  // Point 0
  CountingFrame->GetPoint(0*4+0,point);
  point[2] = -0.1;
  this->MarginPoints->SetPoint(0, point);

  //Point 1
  CountingFrame->GetPoint(0*4+1,point);
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
  CountingFrame->GetPoint(0*4+2,point);
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
  CountingFrame->GetPoint(0*4+3,point);
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
void vtkCountingFrame3DRepresentation::SetCountingFrame(vtkSmartPointer<vtkPolyData> region)
{
  CountingFrame = region;

  //CountingFrame->Update(); NOTE: removed in vtk6, still need something similar?

  VolumePolyData->SetPoints(CountingFrame->GetPoints());
  VolumePolyData->SetPolys(CountingFrame->GetPolys());
  VolumePolyData->GetCellData()->SetScalars(CountingFrame->GetCellData()->GetScalars("Type"));
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::PlaceWidget(double bds[6])
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
int vtkCountingFrame3DRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkCountingFrame3DRepresentation::Outside;
    return this->InteractionState;
    }
  
  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = nullptr;
  this->CurrentHandle = nullptr;
  this->VolumePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->VolumePicker->GetPath();
  if ( path != nullptr )
  {
    this->LastPicker = this->VolumePicker;
    this->ValidPick = 1;
    
    this->CurrentHandle = 
      reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if (this->CurrentHandle == this->MarginActor[LEFT])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveLeft;
    }
    else if (this->CurrentHandle == this->MarginActor[RIGHT])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveRight;
    } 
    else if (this->CurrentHandle == this->MarginActor[TOP])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveTop;
    } 
    else if (this->CurrentHandle == this->MarginActor[BOTTOM])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveBottom;
    }
    else if (this->CurrentHandle == this->MarginActor[UPPER])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveUpper;
    } 
    else if (this->CurrentHandle == this->MarginActor[LOWER])
    {
      this->InteractionState = vtkCountingFrame3DRepresentation::MoveLower;
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    this->InteractionState = vtkCountingFrame3DRepresentation::Outside;
  }
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = ( state < vtkCountingFrame3DRepresentation::Outside ? vtkCountingFrame3DRepresentation::Outside :state);
  
  // Depending on state, highlight appropriate parts of representation
//   int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkCountingFrame3DRepresentation::MoveLeft:
    case vtkCountingFrame3DRepresentation::MoveRight:
    case vtkCountingFrame3DRepresentation::MoveTop:
    case vtkCountingFrame3DRepresentation::MoveBottom:
    case vtkCountingFrame3DRepresentation::MoveUpper:
    case vtkCountingFrame3DRepresentation::MoveLower:
      this->HighlightMargin(this->CurrentHandle);
      break;
    default:
//       this->HighlightOutline(0);
      this->HighlightMargin(nullptr);
//       this->HighlightHandle(nullptr);
//       this->HighlightFace(-1);
      break;
    }
}

//----------------------------------------------------------------------
double *vtkCountingFrame3DRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->VolumeActor->GetBounds());
  return this->BoundingBox->GetBounds();
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::BuildRepresentation()
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
void vtkCountingFrame3DRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->VolumeActor->ReleaseGraphicsResources(w);
  for (unsigned int i=0; i<6; i++)
    this->MarginActor[i]->ReleaseGraphicsResources(w);
//   this->BoundingFaceActor->ReleaseGraphicsResources(w);
//   this->HexFace->ReleaseGraphicsResources(w);

}

//----------------------------------------------------------------------------
int vtkCountingFrame3DRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  count += this->VolumeActor->RenderOpaqueGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkCountingFrame3DRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  count += this->VolumeActor->RenderTranslucentPolygonalGeometry(v);

  return count;
}

//----------------------------------------------------------------------------
int vtkCountingFrame3DRepresentation::HasTranslucentPolygonalGeometry()
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
void vtkCountingFrame3DRepresentation::HighlightMargin(vtkActor* actor)
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
void vtkCountingFrame3DRepresentation::HighlightOutline(int highlight)
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
void vtkCountingFrame3DRepresentation::PrintSelf(ostream& os, vtkIndent indent)
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

