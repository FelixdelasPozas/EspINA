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

// ESPINA
#include "vtkCountingFrame3DRepresentation.h"

// VTK
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkCellData.h>
#include <vtkWindow.h>
#include <vtkCamera.h>

vtkStandardNewMacro(vtkCountingFrame3DRepresentation);

//----------------------------------------------------------------------------
vtkCountingFrame3DRepresentation::vtkCountingFrame3DRepresentation()
: VolumePoints{nullptr}
, Visible     {1}
{
  // The initial state
  CountingFrame = vtkSmartPointer<vtkPolyData>::New();

  Property = vtkSmartPointer<vtkProperty>::New();
  Property->SetRepresentationToSurface();
  Property->SetOpacity(0.7);
  Property->SetDiffuse(1.0);
  Property->SetLineWidth(1.0);

  InclusionLUT = vtkSmartPointer<vtkLookupTable>::New();
  InclusionLUT->SetNumberOfTableValues(2);
  InclusionLUT->Build();
  InclusionLUT->SetTableValue(0,1,0,0);
  InclusionLUT->SetTableValue(1,0,1,0);

  VolumeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VolumeMapper->SetInputData(this->CountingFrame);
  VolumeMapper->SetLookupTable(this->InclusionLUT);

  VolumeActor = vtkSmartPointer<vtkActor>::New();
  VolumeActor->SetMapper(VolumeMapper);
  VolumeActor->SetProperty(Property);
}

//----------------------------------------------------------------------------
vtkCountingFrame3DRepresentation::~vtkCountingFrame3DRepresentation()
{
  CountingFrame->SetPoints(nullptr);
  CountingFrame->SetPolys(nullptr);
  CountingFrame->GetCellData()->SetScalars(nullptr);
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::SetCountingFrame(vtkSmartPointer<vtkPolyData> region)
{
  CountingFrame->SetPoints(region->GetPoints());
  CountingFrame->SetPolys(region->GetPolys());
  CountingFrame->GetCellData()->SetScalars(region->GetCellData()->GetScalars("Type"));
}

//----------------------------------------------------------------------
double *vtkCountingFrame3DRepresentation::GetBounds()
{
  BuildRepresentation();

  return VolumeActor->GetBounds();
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::BuildRepresentation()
{
  // Rebuild only if necessary
  if (this->GetMTime() > this->BuildTime ||
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
  {
    this->VolumeActor->ReleaseGraphicsResources(w);
  }
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
  this->BuildRepresentation();

  return this->VolumeActor->HasTranslucentPolygonalGeometry();
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double bounds[6]{-1,0,-1,0,-1,0};

  if(CountingFrame)
  {
    CountingFrame->GetBounds(bounds);
  }

  os << indent << "Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") " 
     << "(" << bounds[4] << "," << bounds[5] << ")\n";

  if (this->Property)
  {
    os << indent << "Property: " << this->Property << "\n";
  }
  else
  {
    os << indent << "Property: (none)\n";
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::SetVisibility(int visible)
{
  if(Visible != visible)
  {
    Visible = visible;
    VolumeActor->SetVisibility(visible);
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::VisibilityOn()
{
  SetVisibility(true);
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DRepresentation::VisibilityOff()
{
  SetVisibility(false);
}
