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


#include "vtkPVSliceView.h"

#include <QDebug>
#include <assert.h>

#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkDataRepresentation.h>
#include <vtkInteractorStyleImage.h>
#include "vtkLegendScaleActor.h"
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPVGenericRenderWindowInteractor.h>
#include <vtkPVInteractorStyle.h>
#include <vtkRenderer.h>
#include <vtkRenderViewBase.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>

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
  virtual void setCrossHairs(vtkPolyData *hline,
			     vtkPolyData *vline,
			     double center[3],
			     double bounds[6]) = 0;
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
  virtual void setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
			     double center[3], double bounds[6]);
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
void AxialState::setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
			       double center[3], double bounds[6])
{
//   qDebug() << "Crosshair Center: " << center[0] << center[1] << center[2];
  hline->GetPoints()->SetPoint(0,bounds[0],-center[1],0);
  hline->GetPoints()->SetPoint(1,bounds[1],-center[1],0);
  hline->Modified();

  vline->GetPoints()->SetPoint(0,center[0],bounds[2],0);
  vline->GetPoints()->SetPoint(1,center[0],bounds[3],0);
  vline->Modified();
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
  virtual void setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
			     double center[3], double bounds[6]);
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
void SagittalState::setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
				  double center[3], double bounds[6])
{
  hline->GetPoints()->SetPoint(0,0,center[1],bounds[4]);
  hline->GetPoints()->SetPoint(1,0,center[1],bounds[5]);
  hline->Modified();

  vline->GetPoints()->SetPoint(0,0,bounds[2],center[2]);
  vline->GetPoints()->SetPoint(1,0,bounds[3],center[2]);
  vline->Modified();
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
    virtual void setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
			       double center[3], double bounds[6]);

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
void CoronalState::setCrossHairs(vtkPolyData* hline, vtkPolyData* vline,
				 double center[3], double bounds[6])
{
  hline->GetPoints()->SetPoint(0,bounds[0],0,center[2]);
  hline->GetPoints()->SetPoint(1,bounds[1],0,center[2]);
  hline->Modified();

  vline->GetPoints()->SetPoint(0,center[0],0,bounds[4]);
  vline->GetPoints()->SetPoint(1,center[0],0,bounds[5]);
  vline->Modified();
}

//----------------------------------------------------------------------------
void CoronalState::setSlicePosition(vtkMatrix4x4 *matrix, double value)
{
  matrix->SetElement(1, 3, value);
}


//----------------------------------------------------------------------------
// vtkEspinaView
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVSliceView);

//----------------------------------------------------------------------------
vtkPVSliceView::vtkPVSliceView()
{
  bzero(Center,3*sizeof(double));
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

  initCrosshairs();

  SlicingMatrix = vtkMatrix4x4::New();
  SlicingMatrix->DeepCopy(axialSlice);
  SlicingPlane = AXIAL;
  State = AxialState::instance();


  qDebug() << "vtkPVSliceView("<< this << "): Created";
}

//----------------------------------------------------------------------------
void vtkPVSliceView::initCrosshairs()
{
  vtkSmartPointer<vtkPoints> HPoints = vtkSmartPointer<vtkPoints>::New();
  HPoints->InsertNextPoint(LastComputedBounds[0],0,0);
  HPoints->InsertNextPoint(LastComputedBounds[1],0,0);
  vtkSmartPointer<vtkCellArray> HLine = vtkSmartPointer<vtkCellArray>::New();
  HLine->EstimateSize(1,2);
  HLine->InsertNextCell(2);
  HLine->InsertCellPoint(0);
  HLine->InsertCellPoint(1);

  HCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  HCrossLineData->SetPoints(HPoints);
  HCrossLineData->SetLines(HLine);
  HCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInput(HCrossLineData);

  HCrossLine = vtkSmartPointer<vtkActor>::New();
  HCrossLine->SetMapper(HMapper);
//   HCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(HCrossLine);
  OverviewRenderer->AddActor(HCrossLine);
  
  vtkSmartPointer<vtkPoints> VPoints = vtkSmartPointer<vtkPoints>::New();
  VPoints->InsertNextPoint(0,LastComputedBounds[2],0);
  VPoints->InsertNextPoint(0,LastComputedBounds[3],0);
  vtkSmartPointer<vtkCellArray> VLine = vtkSmartPointer<vtkCellArray>::New();
  VLine->EstimateSize(1,2);
  VLine->InsertNextCell(2);
  VLine->InsertCellPoint(0);
  VLine->InsertCellPoint(1);

  VCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  VCrossLineData->SetPoints(VPoints);
  VCrossLineData->SetLines(VLine);
  VCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> VMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VMapper->SetInput(VCrossLineData);

  VCrossLine = vtkSmartPointer<vtkActor>::New();
  VCrossLine->SetMapper(VMapper);
//   VCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(VCrossLine);
  OverviewRenderer->AddActor(VCrossLine);
}


//----------------------------------------------------------------------------
vtkPVSliceView::~vtkPVSliceView()
{
}

//----------------------------------------------------------------------------
void vtkPVSliceView::PrintSelf(ostream& os, vtkIndent indent)
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
void vtkPVSliceView::AddActor(vtkProp3D* actor)
{
  State->updateActor(actor);
  RenderView->GetRenderer()->AddActor(actor);
  OverviewRenderer->AddActor(actor);
  SetCenter(Center);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddChannel(vtkProp3D* actor)
{
  AddActor(actor);
  Channels.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddSegmentation(vtkPVSliceView::SegActor* actor)
{
  AddActor(actor->actor);
  actor->actor->SetVisibility(ShowSegmentations);
  Segmentations.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::Initialize(unsigned int id)
{
    vtkPVRenderView::Initialize(id);
    this->RenderView->GetRenderer()->UseDepthPeelingOff();
    this->OverviewRenderer->UseDepthPeelingOff();

    this->RenderView->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    this->OverviewRenderer->GetActiveCamera()->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCamera()
{
  vtkPVRenderView::ResetCamera();
  OverviewRenderer->ResetCamera(this->LastComputedBounds);
  SetCenter(Center);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCamera(double bounds[6])
{
  vtkPVRenderView::ResetCamera(bounds);
  OverviewRenderer->ResetCamera(bounds);
  SetCenter(Center);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCameraClippingRange()
{
  vtkPVRenderView::ResetCameraClippingRange();
  OverviewRenderer->ResetCameraClippingRange(this->LastComputedBounds);
  SetCenter(Center);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetOrientationAxesVisibility(bool )
{
    vtkPVRenderView::SetOrientationAxesVisibility(true);
}


//----------------------------------------------------------------------------
void vtkPVSliceView::SetBackground(double r, double g, double b)
{
  vtkPVRenderView::SetBackground(r,g,b);
  OverviewRenderer->SetBackground(r,g,b);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetSlice(double pos)
{
//   qDebug() << "vtkPVSliceView " << SlicingPlane << "changing slice" << pos;
  State->setSlicePosition(SlicingMatrix, pos);
  SegActor *seg;
  foreach(seg, Segmentations)
  {
//     double b[6];
//     seg->GetDisplayBounds(b);
//     qDebug() << b[0] << b[1] <<  b[2] <<  b[3] <<  b[4] <<  b[5];
    bool hide = seg->bounds[5] <= Center[SlicingPlane] ||
      seg->bounds[4] > Center[SlicingPlane];
    seg->actor->SetVisibility(!hide && ShowSegmentations);
  }
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetSlicingPlane(int plane)
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
  SegActor *seg;
  foreach(seg, Segmentations)
  {
    State->updateActor(seg->actor);
  }

}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetCenter(double x, double y, double z)
{
//   qDebug() << "SetCenterxyx setting Center" << x << y << z;
//   if (Center[0] == x && Center[1] == y && Center[2] == z)
//     return;

  Center[0] = x;
  Center[1] = y;
  Center[2] = z;
  SetSlice(Center[SlicingPlane]);
  State->setCrossHairs(HCrossLineData,VCrossLineData, Center, LastComputedBounds);
}


//----------------------------------------------------------------------------
void vtkPVSliceView::SetCenter(double center[3])
{
//   qDebug() << "Setting Center3" << center[0] << center[1] << center[2];
  SetCenter(center[0], center[1], center[2]);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetHCrossLineColor(double r, double g, double b)
{
  HCrossLine->GetProperty()->SetColor(r,g,b);
  HCrossLineColor[0] = r;
  HCrossLineColor[1] = g;
  HCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetHCrossLineColor(double color[3])
{
  SetHCrossLineColor(color[0], color[1], color[2]);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetVCrossLineColor(double r, double g, double b)
{
  VCrossLine->GetProperty()->SetColor(r,g,b);
  VCrossLineColor[0] = r;
  VCrossLineColor[1] = g;
  VCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetVCrossLineColor(double color[3])
{
  SetVCrossLineColor(color[0], color[1], color[2]);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetShowSegmentations(bool value)
{
//   qDebug() << "vtkPVSliceView segmentation's visibility = " << value;
  SegActor *seg;
  foreach(seg, Segmentations)
  {
    seg->actor->SetVisibility(value);
  }
  ShowSegmentations = value;
}

