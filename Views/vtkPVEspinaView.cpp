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
#include <assert.h>

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
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>

// Interactor Style to be used with Slice Views
class vtkInteractorStyleEspinaSlice
: public vtkInteractorStyleImage
{
public:
  static vtkInteractorStyleEspinaSlice *New();
  vtkTypeMacro(vtkInteractorStyleEspinaSlice,vtkInteractorStyleImage);

  // Disable mouse wheel
  virtual void OnMouseWheelForward(){}
  virtual void OnMouseWheelBackward(){}

  virtual void OnLeftButtonDown(){}
  virtual void OnLeftButtonUp(){}
//   virtual void OnMouseMove();
protected:
  explicit vtkInteractorStyleEspinaSlice();
  virtual ~vtkInteractorStyleEspinaSlice();

private:
  vtkInteractorStyleEspinaSlice(const vtkInteractorStyleEspinaSlice& ); // Not implemented
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



//-----------------------------------------------------------------------------
// AXIAL STATE
//-----------------------------------------------------------------------------
double axialSlice[16] =
{
  1,  0,  0,  0,
  0,  1,  0,  0,
  0,  0,  1,  0,
  0,  0,  0,  1
};

class EspinaViewState
{
public:
  virtual ~EspinaViewState(){}

  virtual void updateActor(vtkProp3D *actor) = 0;
  virtual void updateCamera(vtkCamera *camera) = 0;
  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix) = 0;
  virtual void setSlicePosition(vtkMatrix4x4 *matrix, double value) = 0;
};

class AxialState : public EspinaViewState
{
public:
  static AxialState *instance()
  {
    if (!m_singleton)
      m_singleton = new AxialState();
    return m_singleton;
  }

  virtual void updateActor(vtkProp3D *actor);
  virtual void updateCamera(vtkCamera* camera);
  virtual void updateSlicingMatrix(vtkMatrix4x4 *matrix);
  virtual void setSlicePosition(vtkMatrix4x4 *matrix, double value);

protected:
  AxialState(){}

private:
  static AxialState *m_singleton;
};

//-----------------------------------------------------------------------------
AxialState *AxialState::m_singleton = NULL;

//-----------------------------------------------------------------------------
void AxialState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(180);
}

//-----------------------------------------------------------------------------
void AxialState::updateCamera(vtkCamera* camera)
{
  camera->SetPosition(0, 0, -1);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetRoll(180);
}

//-----------------------------------------------------------------------------
void AxialState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(axialSlice);
}

//-----------------------------------------------------------------------------
void AxialState::setSlicePosition(vtkMatrix4x4 *matrix, double value)
{
  matrix->SetElement(2, 3, value);
}


//-----------------------------------------------------------------------------
// SAGITTAL STATE
//-----------------------------------------------------------------------------
double saggitalSlice[16] =
{
  0,  0, -1,  0,
  1,  0,  0,  0,
  0, -1,  0,  0,
  0,  0,  0,  1
};

class SagittalState : public EspinaViewState
{
public:
  static SagittalState *instance()
  {
    if (!m_singleton)
      m_singleton = new SagittalState();
    return m_singleton;
  }

  virtual void updateActor(vtkProp3D* actor);
  virtual void updateCamera(vtkCamera* camera);
  virtual void updateSlicingMatrix(vtkMatrix4x4* matrix);
  virtual void setSlicePosition(vtkMatrix4x4* matrix, double value);

protected:
  SagittalState(){}

private:
  static SagittalState *m_singleton;
};

//-----------------------------------------------------------------------------
SagittalState *SagittalState::m_singleton = NULL;

//----------------------------------------------------------------------------
void SagittalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
  actor->RotateY(-90);
}

//----------------------------------------------------------------------------
void SagittalState::updateCamera(vtkCamera* camera)
{
  camera->SetPosition(1, 0, 0);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetRoll(180);
}

//----------------------------------------------------------------------------
void SagittalState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(saggitalSlice);
}

//-----------------------------------------------------------------------------
void SagittalState::setSlicePosition(vtkMatrix4x4 *matrix, double value)
{
  matrix->SetElement(0, 3, value);
}


//-----------------------------------------------------------------------------
// CORONAL STATE
//-----------------------------------------------------------------------------
double coronalSlice[16] =
{
  1,  0,  0,  0,
  0,  0,  1,  0,
  0, -1,  0,  0,
  0,  0,  0,  1
};

class CoronalState : public EspinaViewState
{
public:
  static CoronalState *instance()
  {
    if (!m_singleton)
      m_singleton = new CoronalState();
    return m_singleton;
  }

    virtual void updateActor(vtkProp3D* actor);
    virtual void updateCamera(vtkCamera* camera);
    virtual void updateSlicingMatrix(vtkMatrix4x4* matrix);
    virtual void setSlicePosition(vtkMatrix4x4* matrix, double value);

protected:
  CoronalState(){}

private:
  static CoronalState *m_singleton;
};

//-----------------------------------------------------------------------------
CoronalState *CoronalState::m_singleton = NULL;

//----------------------------------------------------------------------------
void CoronalState::updateActor(vtkProp3D* actor)
{
  actor->RotateX(-90);
}

//----------------------------------------------------------------------------
void CoronalState::updateCamera(vtkCamera* camera)
{
  camera->Roll(90);
  camera->Azimuth(90);
  camera->Roll(90);
  camera->Elevation(180);

}

//----------------------------------------------------------------------------
void CoronalState::updateSlicingMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(coronalSlice);
}

//----------------------------------------------------------------------------
void CoronalState::setSlicePosition(vtkMatrix4x4 *matrix, double value)
{
  matrix->SetElement(1, 3, value);
}


//----------------------------------------------------------------------------
// vtkEspinaView
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVEspinaView);

//----------------------------------------------------------------------------

vtkPVEspinaView::vtkPVEspinaView()
{
  this->SetCenterAxesVisibility(false);
  this->SetOrientationAxesVisibility(false);
  this->SetOrientationAxesInteractivity(false);
  this->SetInteractionMode(INTERACTION_MODE_3D);

  if (this->Interactor)
  {
//     vtkInteractorStyleImage *style = vtkInteractorStyleImage::New();
    vtkInteractorStyleEspinaSlice *style = vtkInteractorStyleEspinaSlice::New();
    this->Interactor->SetInteractorStyle(style);
  }

  RenderView->GetRenderer()->SetLayer(0);
  this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->OverviewRenderer->SetViewport(0.75,0,1,0.25);
  OverviewRenderer->SetLayer(1);
  this->GetRenderWindow()->AddRenderer(this->OverviewRenderer);

  SlicingMatrix = vtkMatrix4x4::New();
  SlicingMatrix->DeepCopy(axialSlice);
  SlicingPlane = AXIAL;
  State = AxialState::instance();

  qDebug() << "vtkPVEspinaView("<< this << "): Created";
}

//----------------------------------------------------------------------------
vtkPVEspinaView::~vtkPVEspinaView()
{
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OverviewRenderer: ";
  if (this->OverviewRenderer)
  {
    os << "\n";
    this->OverviewRenderer->PrintSelf(os, indent.GetNextIndent());
  }
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::AddActor(vtkProp3D* actor)
{
  State->updateActor(actor);
  RenderView->GetRenderer()->AddActor(actor);
  OverviewRenderer->AddActor(actor);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::AddChannel(vtkProp3D* actor)
{
  AddActor(actor);
  Channels.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::AddSegmentation(vtkProp3D* actor)
{
  AddActor(actor);
  actor->SetVisibility(ShowSegmentations);
  Segmentations.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::Initialize(unsigned int id)
{
    vtkPVRenderView::Initialize(id);
    this->RenderView->GetRenderer()->UseDepthPeelingOff();
    this->OverviewRenderer->UseDepthPeelingOff();

    this->RenderView->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    this->OverviewRenderer->GetActiveCamera()->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCamera()
{
  vtkPVRenderView::ResetCamera();
  OverviewRenderer->ResetCamera(this->LastComputedBounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCamera(double bounds[6])
{
  vtkPVRenderView::ResetCamera(bounds);
  OverviewRenderer->ResetCamera(bounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::ResetCameraClippingRange()
{
    vtkPVRenderView::ResetCameraClippingRange();
    OverviewRenderer->ResetCameraClippingRange(this->LastComputedBounds);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetOrientationAxesVisibility(bool )
{
    vtkPVRenderView::SetOrientationAxesVisibility(true);
}


//----------------------------------------------------------------------------
void vtkPVEspinaView::SetBackground(double r, double g, double b)
{
  vtkPVRenderView::SetBackground(r,g,b);
  OverviewRenderer->SetBackground(r,g,b);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetSlice(int value)
{
//   qDebug() << "vtkPVEspinaView changing slice" << value;
  State->setSlicePosition(SlicingMatrix,value);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetSlicingPlane(int plane)
{
  if (SlicingPlane == plane)
    return;

  SlicingPlane = static_cast<VIEW_PLANE>(plane);

  switch (SlicingPlane)
  {
    case AXIAL:
      State = AxialState::instance();
      break;
    case SAGITTAL:
      State = SagittalState::instance();
      break;
    case CORONAL:
    default:
      State = CoronalState::instance();
  };

  State->updateSlicingMatrix(SlicingMatrix);
  State->updateCamera(RenderView->GetRenderer()->GetActiveCamera());
  State->updateCamera(OverviewRenderer->GetActiveCamera());

  vtkProp3D *actor;
  foreach(actor, Channels)
  {
    State->updateActor(actor);
  }
  foreach(actor, Segmentations)
  {
    State->updateActor(actor);
  }

}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetCenter(double pos[3])
{
  
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetShowSegmentations(bool value)
{
//   qDebug() << "vtkPVEspinaView segmentation's visibility = " << value;
  vtkProp3D *seg;
  foreach(seg, Segmentations)
  {
    seg->SetVisibility(value);
  }
  ShowSegmentations = value;
}

