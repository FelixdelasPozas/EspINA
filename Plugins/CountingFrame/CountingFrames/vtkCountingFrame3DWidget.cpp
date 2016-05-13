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
#include "vtkCountingFrame3DWidget.h"
#include "CountingFrames/vtkCountingFrame3DRepresentation.h"

// VTK
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

vtkStandardNewMacro(vtkCountingFrame3DWidget);

//----------------------------------------------------------------------------
vtkCountingFrame3DWidget::vtkCountingFrame3DWidget()
{
  this->WidgetState = vtkCountingFrame3DWidget::Start;
  this->ManagesCursor = 0;

  memset(InclusionOffset, 0, 3*sizeof(double));
  memset(ExclusionOffset, 0, 3*sizeof(double));
}

//----------------------------------------------------------------------------
vtkCountingFrame3DWidget::~vtkCountingFrame3DWidget()
{
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::SetCountingFrame(vtkSmartPointer< vtkPolyData > region, ESPINA::Nm inclusionOffset[3], ESPINA::Nm exclusionOffset[3], ESPINA::NmVector3 resolution)
{
  if (!this->WidgetRep)
  {
    CreateDefaultRepresentation();
  }

  auto rep = reinterpret_cast<vtkCountingFrame3DRepresentation*>(this->WidgetRep);
  rep->SetCountingFrame(region);

  this->Render();
}

//----------------------------------------------------------------------
void vtkCountingFrame3DWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkCountingFrame3DRepresentation::New();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DWidget::setVisible(bool visible)
{
  if(Visible != visible)
  {
    Visible = visible;

    auto rep = reinterpret_cast<vtkCountingFrame3DRepresentation*>(this->WidgetRep);
    rep->SetVisibility(visible);

    this->Render();
  }
}

//----------------------------------------------------------------------------
void vtkCountingFrame3DWidget::SetOpacity(const float opacity)
{
  auto rep = reinterpret_cast<vtkCountingFrame3DRepresentation*>(this->WidgetRep);
  if(opacity != rep->opacity())
  {
    rep->setOpacity(opacity);

    this->Render();
  }
}

//----------------------------------------------------------------------------
const float vtkCountingFrame3DWidget::GetOpacity() const
{
  auto rep = reinterpret_cast<vtkCountingFrame3DRepresentation*>(this->WidgetRep);
  return rep->opacity();
}
