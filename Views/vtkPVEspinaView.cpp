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


#include "vtkPVEspinaView.h"

#include <QDebug>

#include "vtkEspinaView.h"

#include "vtkObjectFactory.h"
#include "vtkLegendScaleActor.h"
#include "vtkRenderer.h"
#include <vtkDataRepresentation.h>
#include <vtkCommand.h>

#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindow.h>
#include <vtkPVGenericRenderWindowInteractor.h>
#include <vtkPVInteractorStyle.h>

// Interactor Style to be used with Slice Views
class vtkInteractorStyleEspinaSlice
: public vtkPVInteractorStyle
{
public:
  static vtkInteractorStyleEspinaSlice *New();
  vtkTypeMacro(vtkInteractorStyleEspinaSlice,vtkPVInteractorStyle);

  // Disable mouse wheel
  virtual void OnMouseWheelForward(){}
  virtual void OnMouseWheelBackward(){}
//   virtual void OnMouseMove();
protected:
  explicit vtkInteractorStyleEspinaSlice();
  virtual ~vtkInteractorStyleEspinaSlice();

private:
  vtkInteractorStyleEspinaSlice(const vtkInteractorStyleImage& ); // Not implemented
  void operator=(const vtkInteractorStyleEspinaSlice&);           // Not implemented
};

vtkStandardNewMacro(vtkInteractorStyleEspinaSlice);

//-----------------------------------------------------------------------------
vtkInteractorStyleEspinaSlice::vtkInteractorStyleEspinaSlice()
{
}

//-----------------------------------------------------------------------------
vtkInteractorStyleEspinaSlice::~vtkInteractorStyleEspinaSlice()
{
  qDebug() << "vtkInteractorStyleEspinaSlice(" << this << "): Destroyed";
}


vtkStandardNewMacro(vtkPVEspinaView);
//----------------------------------------------------------------------------

vtkPVEspinaView::vtkPVEspinaView()
{
  this->SetCenterAxesVisibility(false);
  this->SetOrientationAxesVisibility(false);
  this->SetOrientationAxesInteractivity(false);
  this->SetInteractionMode(INTERACTION_MODE_3D);

  vtkRenderViewBase *oldRV = RenderView;
  EspinaView = vtkEspinaView::New();
  this->RenderView = EspinaView;
  this->RenderView->SetInteractor(oldRV->GetInteractor());
  this->RenderView->SetRenderWindow(oldRV->GetRenderWindow());
  this->RenderView->SetInteractor(oldRV->GetInteractor());
//   EspinaView->GetOverviewRenderer()->SetActiveCamera(RenderView->GetRenderer()->GetActiveCamera());
  oldRV->Delete();
  if (this->Interactor)
  {
    this->InteractorStyle = vtkInteractorStyleEspinaSlice::New();
    this->Interactor->SetRenderer(this->GetRenderer());
    this->Interactor->SetRenderWindow(this->GetRenderWindow());
    this->Interactor->SetInteractorStyle(this->InteractorStyle);
  }
//   this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
//   this->OverviewRenderer->SetViewport(0.75,0,1,0.25);
//   this->GetRenderWindow()->AddRenderer(this->OverviewRenderer);

  qDebug() << this << ": Created";
}

//----------------------------------------------------------------------------
vtkPVEspinaView::~vtkPVEspinaView()
{
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::AddActor(vtkProp* actor)
{
//   EspinaView->GetRenderer()->SetBackground2(1,0,0);
  EspinaView->AddActor(actor);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCamera()
{
  vtkPVRenderView::ResetCamera();
  EspinaView->GetOverviewRenderer()->ResetCamera(this->LastComputedBounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCamera(double bounds[6])
{
  vtkPVRenderView::ResetCamera(bounds);
  EspinaView->GetOverviewRenderer()->ResetCamera(bounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCameraClippingRange()
{
    vtkPVRenderView::ResetCameraClippingRange();
    EspinaView->GetOverviewRenderer()->ResetCameraClippingRange(this->LastComputedBounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetOrientationAxesVisibility(bool )
{
    vtkPVRenderView::SetOrientationAxesVisibility(false);
}


//----------------------------------------------------------------------------
void vtkPVEspinaView::SetBackground(double r, double g, double b)
{
  vtkPVRenderView::SetBackground(r,g,b);
  EspinaView->GetOverviewRenderer()->SetBackground(r,g,b);
}


