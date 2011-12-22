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


#include "vtkEspinaView.h"

#include <QDebug>
#include <assert.h>

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <vtkPVInteractorStyle.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkEspinaView);

//----------------------------------------------------------------------------
vtkEspinaView::vtkEspinaView()
{
  std::cout << "Dir of Render Window in subclass:" << &this->RenderWindow << std::endl;
  assert(this->RenderWindow);

  this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->OverviewRenderer->SetViewport(0.75,0,1,0.25);
  this->RenderWindow->AddRenderer(this->OverviewRenderer);

//   vtkPVInteractorStyle *style = vtkPVInteractorStyle::New();
//   this->RenderWindow->GetInteractor()->SetInteractorStyle(style);
}

//----------------------------------------------------------------------------
vtkEspinaView::~vtkEspinaView()
{
}

//----------------------------------------------------------------------------
vtkRenderer* vtkEspinaView::GetOverviewRenderer()
{
  return this->OverviewRenderer;
}

//----------------------------------------------------------------------------
void vtkEspinaView::SetOverviewRenderer(vtkRenderer* ren)
{
  assert(false);
  this->OverviewRenderer = ren;
}

//----------------------------------------------------------------------------
void vtkEspinaView::AddActor(vtkProp* actor)
{
  this->Renderer->AddActor(actor);
  this->OverviewRenderer->AddActor(actor);
}



//----------------------------------------------------------------------------
void vtkEspinaView::setSlice(unsigned int val)
{
}

//----------------------------------------------------------------------------
void vtkEspinaView::ResetCamera()
{
  Superclass::ResetCamera();
  OverviewRenderer->ResetCamera();
}

//----------------------------------------------------------------------------
void vtkEspinaView::ResetCameraClippingRange()
{
    Superclass::ResetCameraClippingRange();
    OverviewRenderer->ResetCameraClippingRange();
}


//----------------------------------------------------------------------------
void vtkEspinaView::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkRenderViewBase::PrintSelf(os, indent);
  os << indent << "OverviewRenderer: ";
  if (this->OverviewRenderer)
  {
    os << "\n";
    this->OverviewRenderer->PrintSelf(os, indent.GetNextIndent());
  }
}


