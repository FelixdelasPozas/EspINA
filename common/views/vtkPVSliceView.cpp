/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "vtkPVSliceView.h"

#include <QDebug>
#include <assert.h>

#include <vtkAbstractPicker.h>
#include <vtkAxisActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkDataRepresentation.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLegendScaleActor.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkPVGenericRenderWindowInteractor.h>
#include <vtkPVInteractorStyle.h>
#include <vtkPVSynchronizedRenderWindows.h>
#include <vtkRenderer.h>
#include <vtkRenderViewBase.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkPropPicker.h>

// Interactor Style to be used with Slice Views
class vtkInteractorStyleEspinaSlice
: public vtkInteractorStyleImage
{
public:
  static vtkInteractorStyleEspinaSlice *New();
  vtkTypeMacro ( vtkInteractorStyleEspinaSlice,vtkInteractorStyleImage );

  // Disable mouse wheel
  virtual void OnMouseWheelForward() {}
  virtual void OnMouseWheelBackward() {}

  // Disable modifying brightness and saturation
  virtual void OnLeftButtonDown() { }
  virtual void OnLeftButtonUp() {}
  //   virtual void OnMouseMove();
protected:
  explicit vtkInteractorStyleEspinaSlice();
  virtual ~vtkInteractorStyleEspinaSlice();

private:
  vtkInteractorStyleEspinaSlice ( const vtkInteractorStyleEspinaSlice& ); // Not implemented
  void operator= ( const vtkInteractorStyleEspinaSlice& );        // Not implemented
};

vtkStandardNewMacro ( vtkInteractorStyleEspinaSlice );

//-----------------------------------------------------------------------------
vtkInteractorStyleEspinaSlice::vtkInteractorStyleEspinaSlice()
{
}

//-----------------------------------------------------------------------------
vtkInteractorStyleEspinaSlice::~vtkInteractorStyleEspinaSlice()
{
//   qDebug() << "vtkInteractorStyleEspinaSlice(" << this << "): Destroyed";
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

double origin[3] = {0, 0, 0};

class EspinaViewState
{
public:
  virtual ~EspinaViewState() {}

  virtual void updateActor ( vtkProp3D *actor ) = 0;
  virtual void updateCamera (vtkCamera *camera, double center[3] = origin) = 0;
  virtual void updateSlicingMatrix ( vtkMatrix4x4 *matrix ) = 0;
  virtual void setCrossHairs ( vtkPolyData *hline,
			       vtkPolyData *vline,
			       double center[3],
			       double bounds[6] ) = 0;
			       virtual void setSlicePosition ( vtkMatrix4x4 *matrix, double value ) = 0;
};

class AxialState : public EspinaViewState
{
public:
  static AxialState *instance()
  {
    if ( !m_singleton )
      m_singleton = new AxialState();
    return m_singleton;
  }

  virtual void updateActor(vtkProp3D *actor);
  virtual void updateCamera(vtkCamera *camera, double center[3] = origin);
  virtual void updateSlicingMatrix ( vtkMatrix4x4 *matrix );
  virtual void setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
			       double center[3], double bounds[6] );
  virtual void setSlicePosition ( vtkMatrix4x4 *matrix, double value );

protected:
  AxialState() {}

private:
  static AxialState *m_singleton;
};

//-----------------------------------------------------------------------------
AxialState *AxialState::m_singleton = NULL;

//-----------------------------------------------------------------------------
void AxialState::updateActor ( vtkProp3D* actor )
{
  //   actor->RotateX(180);
}

//-----------------------------------------------------------------------------
void AxialState::updateCamera (vtkCamera* camera, double center[3])
{
  camera->SetPosition (center[0], center[1], -1);
  camera->SetFocalPoint(center[0], center[1], 0);
  camera->SetRoll(180);
  camera->ParallelProjectionOn();
}

//-----------------------------------------------------------------------------
void AxialState::updateSlicingMatrix ( vtkMatrix4x4* matrix )
{
  matrix->DeepCopy(axialSlice);
}

//-----------------------------------------------------------------------------
void AxialState::setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
				 double center[3], double bounds[6] )
{
  //   qDebug() << "Crosshair Center: " << center[0] << center[1] << center[2];
  hline->GetPoints()->SetPoint ( 0,bounds[0],center[1],0 );
  hline->GetPoints()->SetPoint ( 1,bounds[1],center[1],0 );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,center[0],bounds[2],0 );
  vline->GetPoints()->SetPoint ( 1,center[0],bounds[3],0 );
  vline->Modified();
}


//-----------------------------------------------------------------------------
void AxialState::setSlicePosition ( vtkMatrix4x4 *matrix, double value )
{
  matrix->SetElement(2, 3, value);
}


//-----------------------------------------------------------------------------
// SAGITTAL STATE
//-----------------------------------------------------------------------------
double sagittalSlice[16] =
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
    if ( !m_singleton )
      m_singleton = new SagittalState();
    return m_singleton;
  }

  virtual void updateActor ( vtkProp3D* actor );
  virtual void updateCamera(vtkCamera *camera, double center[3] = origin);
  virtual void updateSlicingMatrix ( vtkMatrix4x4* matrix );
  virtual void setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
			       double center[3], double bounds[6] );
  virtual void setSlicePosition ( vtkMatrix4x4* matrix, double value );

protected:
  SagittalState() {}

private:
  static SagittalState *m_singleton;
};

//-----------------------------------------------------------------------------
SagittalState *SagittalState::m_singleton = NULL;

//----------------------------------------------------------------------------
void SagittalState::updateActor ( vtkProp3D* actor )
{
  actor->RotateX(-90);
  actor->RotateY(-90);
}

//----------------------------------------------------------------------------
void SagittalState::updateCamera(vtkCamera* camera, double center[3])
{
  //   qDebug() << "Update sagittal Camera";
  camera->SetPosition(center[0] + 1,center[1], center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void SagittalState::updateSlicingMatrix ( vtkMatrix4x4* matrix )
{
  matrix->DeepCopy ( sagittalSlice );
}

//-----------------------------------------------------------------------------
void SagittalState::setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
				    double center[3], double bounds[6] )
{
  hline->GetPoints()->SetPoint ( 0,0,center[1],bounds[4] );
  hline->GetPoints()->SetPoint ( 1,0,center[1],bounds[5] );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,0,bounds[2],center[2] );
  vline->GetPoints()->SetPoint ( 1,0,bounds[3],center[2] );
  vline->Modified();
}

//-----------------------------------------------------------------------------
void SagittalState::setSlicePosition ( vtkMatrix4x4 *matrix, double value )
{
  matrix->SetElement ( 0, 3, value );
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
    if ( !m_singleton )
      m_singleton = new CoronalState();
    return m_singleton;
  }

  virtual void updateActor(vtkProp3D* actor);
  virtual void updateCamera(vtkCamera* camera, double center[3] = origin);
  virtual void updateSlicingMatrix ( vtkMatrix4x4* matrix );
  virtual void setSlicePosition ( vtkMatrix4x4* matrix, double value );
  virtual void setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
			       double center[3], double bounds[6] );

protected:
  CoronalState() {}

private:
  static CoronalState *m_singleton;
};

//-----------------------------------------------------------------------------
CoronalState *CoronalState::m_singleton = NULL;

//----------------------------------------------------------------------------
void CoronalState::updateActor ( vtkProp3D* actor )
{
  actor->RotateX ( -90 );
}

//----------------------------------------------------------------------------
void CoronalState::updateCamera(vtkCamera* camera, double center[3])
{
  //   qDebug() << "Update coronal Camera";
  camera->SetPosition(center[0], center[1]+1, center[2]);
  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetViewUp(0, 0, -1);
  camera->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void CoronalState::updateSlicingMatrix ( vtkMatrix4x4* matrix )
{
  matrix->DeepCopy ( coronalSlice );
}

//----------------------------------------------------------------------------
void CoronalState::setCrossHairs ( vtkPolyData* hline, vtkPolyData* vline,
				   double center[3], double bounds[6] )
{
  hline->GetPoints()->SetPoint ( 0,bounds[0],0,center[2] );
  hline->GetPoints()->SetPoint ( 1,bounds[1],0,center[2] );
  hline->Modified();

  vline->GetPoints()->SetPoint ( 0,center[0],0,bounds[4] );
  vline->GetPoints()->SetPoint ( 1,center[0],0,bounds[5] );
  vline->Modified();
}

//----------------------------------------------------------------------------
void CoronalState::setSlicePosition ( vtkMatrix4x4 *matrix, double value )
{
  matrix->SetElement ( 1, 3, value );
}


//----------------------------------------------------------------------------
// vtkEspinaView
//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkPVSliceView );

//----------------------------------------------------------------------------
vtkPVSliceView::vtkPVSliceView()
: m_pendingActor(NULL)
{
  memset(Center, 0, 3*sizeof(double));

  this->SetCenterAxesVisibility ( false );
  this->SetOrientationAxesVisibility ( false );
  this->SetOrientationAxesInteractivity ( false );
  this->SetInteractionMode ( INTERACTION_MODE_3D );

  if ( this->Interactor )
  {
    //     vtkInteractorStyleImage *style = vtkInteractorStyleImage::New();
    vtkInteractorStyleEspinaSlice *style = vtkInteractorStyleEspinaSlice::New();
    this->Interactor->SetInteractorStyle(style);
    style->Delete();
  }

  RenderView->GetRenderer()->SetLayer(0);
  NonCompositedRenderer->SetLayer(1);
  this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->OverviewRenderer->SetViewport(0.75, 0, 1, 0.25);
  OverviewRenderer->SetLayer(2);
  this->GetRenderWindow()->AddRenderer(this->OverviewRenderer);

  initCrosshairs();
  initRuler();
  initBorder(SliceBorderData, SliceBorder);
  initBorder(ViewBorderData,  ViewBorder);
//   initViewBorder();

  SlicingMatrix = vtkMatrix4x4::New();
  SlicingMatrix->DeepCopy ( axialSlice );
  SlicingPlane = AXIAL;
  State = AxialState::instance();

  ChannelPicker = vtkSmartPointer<vtkPropPicker>::New();
  ChannelPicker->PickFromListOn();

  SegmentationPicker = vtkSmartPointer<vtkPropPicker>::New();
  SegmentationPicker->PickFromListOn();

  //     qDebug() << "vtkPVSliceView("<< this << "): Created";
}

//----------------------------------------------------------------------------
void vtkPVSliceView::initCrosshairs()
{
  vtkSmartPointer<vtkPoints> HPoints = vtkSmartPointer<vtkPoints>::New();
  HPoints->InsertNextPoint(LastComputedBounds[0], 0, 0);
  HPoints->InsertNextPoint(LastComputedBounds[1], 0, 0);
  vtkSmartPointer<vtkCellArray> HLine = vtkSmartPointer<vtkCellArray>::New();
  HLine->EstimateSize(1, 2);
  HLine->InsertNextCell (2);
  HLine->InsertCellPoint(0);
  HLine->InsertCellPoint(1);

  HCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  HCrossLineData->SetPoints(HPoints);
  HCrossLineData->SetLines (HLine);
  HCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInput(HCrossLineData);

  HCrossLine = vtkSmartPointer<vtkActor>::New();
  HCrossLine->SetMapper(HMapper);
  HCrossLine->SetPickable(false);
  //   HCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(HCrossLine);
  OverviewRenderer->AddActor(HCrossLine);

  vtkSmartPointer<vtkPoints> VPoints = vtkSmartPointer<vtkPoints>::New();
  VPoints->InsertNextPoint(0, LastComputedBounds[2], 0);
  VPoints->InsertNextPoint(0, LastComputedBounds[3], 0);
  vtkSmartPointer<vtkCellArray> VLine = vtkSmartPointer<vtkCellArray>::New();
  VLine->EstimateSize(1, 2);
  VLine->InsertNextCell (2);
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
  VCrossLine->SetPickable(false);
  //   VCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);

  NonCompositedRenderer->AddActor(VCrossLine);
  OverviewRenderer->AddActor(VCrossLine);

  //   vtkSmartPointer<vtkPoints> BPoints = vtkSmartPointer<vtkPoints>::New();
  //   BPoints->InsertNextPoint(LastComputedBounds[0],LastComputedBounds[2],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[0],LastComputedBounds[3],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[1],LastComputedBounds[3],0);
  //   BPoints->InsertNextPoint(LastComputedBounds[1],LastComputedBounds[2],0);
  //   vtkSmartPointer<vtkCellArray> BLine = vtkSmartPointer<vtkCellArray>::New();
  //   BLine->EstimateSize(4,2);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(0);
  //   BLine->InsertCellPoint(1);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(1);
  //   BLine->InsertCellPoint(2);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(2);
  //   BLine->InsertCellPoint(3);
  //   BLine->InsertNextCell(2);
  //   BLine->InsertCellPoint(3);
  //   BLine->InsertCellPoint(0);
  //
  //   BorderData = vtkSmartPointer<vtkPolyData>::New();
  //   BorderData->SetPoints(BPoints);
  //   BorderData->SetLines(BLine);
  //   BorderData->Update();
  //
  //   vtkSmartPointer<vtkPolyDataMapper> BMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  //   BMapper->SetInput(BorderData);
  //
  //   Border = vtkSmartPointer<vtkActor>::New();
  //   Border->SetMapper(BMapper);
  //   Border->SetPickable(false);
  //   //   HCrossLine->GetProperty()->SetLineStipplePattern(0xF0F0);
  //
  //   NonCompositedRenderer->AddActor(Border);
  //   OverviewRenderer->AddActor(Border);

}

//----------------------------------------------------------------------------
void vtkPVSliceView::initRuler()
{
  RulerSize[0] = 150;
  RulerSize[1] = 2;
  Ruler = vtkSmartPointer<vtkAxisActor2D>::New();
  Ruler->SetPosition ( 0.02,0.98 );
  Ruler->SetPosition2 ( 0.15,0.98 );
  Ruler->SetPickable ( false );
  Ruler->SetLabelFactor ( 0.8 );
  Ruler->SetFontFactor ( 1 );
  Ruler->SetTitle ( "nm" );
  Ruler->SetAdjustLabels ( false );
  Ruler->SetNumberOfLabels ( 3 );
  this->NonCompositedRenderer->AddActor ( Ruler );
  //   RulerRenderer->ResetCamera();
}

//----------------------------------------------------------------------------
void vtkPVSliceView::initBorder(vtkSmartPointer<vtkPolyData> &data,
				vtkSmartPointer<vtkActor> &actor)
{
  vtkSmartPointer<vtkPoints> corners = vtkSmartPointer<vtkPoints>::New();
  corners->InsertNextPoint(LastComputedBounds[0], LastComputedBounds[2], 0); //UL
  corners->InsertNextPoint(LastComputedBounds[0], LastComputedBounds[3], 0); //UR
  corners->InsertNextPoint(LastComputedBounds[1], LastComputedBounds[3], 0); //LR
  corners->InsertNextPoint(LastComputedBounds[1], LastComputedBounds[2], 0); //LL
  vtkSmartPointer<vtkCellArray> borders = vtkSmartPointer<vtkCellArray>::New();
  borders->EstimateSize(4, 2);
  for (int i=0; i < 4; i++)
  {
    borders->InsertNextCell (2);
    borders->InsertCellPoint(i);
    borders->InsertCellPoint((i+1)%4);
  }
  data = vtkSmartPointer<vtkPolyData>::New();
  data->SetPoints(corners);
  data->SetLines(borders);
  data->Update();

  vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  Mapper->SetInput(data);
  actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(Mapper);
  actor->GetProperty()->SetLineWidth(2);
  actor->SetPickable(false);

  OverviewRenderer->AddActor(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::Render(bool interactive, bool skip_rendering)
{
  updateRuler();
  updateThumbnail();
  vtkPVRenderView::Render(interactive, skip_rendering);
}

//----------------------------------------------------------------------------
vtkPVSliceView::~vtkPVSliceView()
{
  SlicingMatrix->Delete();
  //   qDebug() << "vtkPVSliceView("<< this << "): Deleted";
}

//----------------------------------------------------------------------------
void vtkPVSliceView::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "OverviewRenderer: ";
  if ( this->OverviewRenderer )
  {
    os << "\n";
    this->OverviewRenderer->PrintSelf ( os, indent.GetNextIndent() );
  }
}

//----------------------------------------------------------------------------
vtkRenderer* vtkPVSliceView::GetOverviewRenderer()
{
  return OverviewRenderer.GetPointer();
}

//----------------------------------------------------------------------------
vtkPropPicker* vtkPVSliceView::GetChannelPicker()
{
  return ChannelPicker.GetPointer();
}

//----------------------------------------------------------------------------
vtkPropPicker* vtkPVSliceView::GetSegmentationPicker()
{
  return SegmentationPicker.GetPointer();
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddActor(SliceActor *actor)
{
  //   qDebug() << "Add Actor";
  bool in_cave_mode = this->SynchronizedWindows->GetIsInCave();
  if ( in_cave_mode && !this->GetRemoteRenderingAvailable() )
  {
    static bool warned_once = false;
    if ( !warned_once )
    {
      vtkErrorMacro (
	"In Cave mode and Display cannot be opened on server-side! "
	"Ensure the environment is set correctly in the pvx file." );
      in_cave_mode = true;
    }
  }

  // Decide if we are doing remote rendering or local rendering.
  bool using_distributed_rendering = in_cave_mode || this->GetUseDistributedRendering();
  if ( this->GetLocalProcessDoesRendering (using_distributed_rendering))
    RenderView->GetInteractor()->GetPicker()->AddPickList(actor->prop);

  State->updateActor(actor->prop);
  RenderView->GetRenderer()->AddActor(actor->prop);
  OverviewRenderer->AddActor(actor->prop);
  SetCenter(Center);
  m_actors << actor;
  Q_ASSERT(!m_pendingActor);
  m_pendingActor = actor;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::RemoveActor(SliceActor *actor)
{
  RenderView->GetRenderer()->RemoveActor(actor->prop);
  OverviewRenderer->RemoveActor(actor->prop);
  ChannelPicker->DeletePickList(actor->prop);
  SegmentationPicker->DeletePickList(actor->prop);
  m_actors.removeOne(actor);
  if (Segmentations.contains(actor))
    Segmentations.removeOne(actor);
  if (Channels.contains(actor))
    Channels.removeOne(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddChannel(SliceActor* actor)
{
  bool in_cave_mode = this->SynchronizedWindows->GetIsInCave();
  // Decide if we are doing remote rendering or local rendering.
  bool using_distributed_rendering = in_cave_mode || this->GetUseDistributedRendering();
  if ( this->GetLocalProcessDoesRendering (using_distributed_rendering))
    ChannelPicker->AddPickList(actor->prop);

  AddActor(actor);
  Channels.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddSegmentation(vtkPVSliceView::SliceActor* actor)
{
  //   qDebug() << "Add Segmentation";
  bool in_cave_mode = this->SynchronizedWindows->GetIsInCave();
  // Decide if we are doing remote rendering or local rendering.
  bool using_distributed_rendering = in_cave_mode || this->GetUseDistributedRendering();
  if ( this->GetLocalProcessDoesRendering (using_distributed_rendering))
    SegmentationPicker->AddPickList(actor->prop);

  AddActor(actor);
  actor->prop->SetVisibility(ShowSegmentations);
  double pos[3];
  actor->prop->GetPosition(pos);
  pos[2] = pos[2] - 0.2;
  actor->prop->SetPosition(pos);
  Segmentations.append(actor);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::AddRepresentationInternal(vtkDataRepresentation* rep)
{
  //   qDebug() << "ADDING REPRESENTATION";
  if (m_pendingActor)
  {
    m_reps[rep] = m_pendingActor;
    m_pendingActor = NULL;
  }
  vtkPVRenderView::AddRepresentationInternal(rep);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::RemoveRepresentationInternal(vtkDataRepresentation* rep)
{
  //   qDebug() << "REMOVING REPRESENTATION";
  SliceActor *actor = m_reps.value(rep, NULL);
  if (actor)
  {
    RemoveActor(actor);
    m_reps.remove(rep);
  }
  vtkPVRenderView::RemoveRepresentationInternal(rep);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::Initialize ( unsigned int id )
{
  vtkPVRenderView::Initialize ( id );
  this->RenderView->GetRenderer()->UseDepthPeelingOff();
  this->OverviewRenderer->UseDepthPeelingOff();

  this->RenderView->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
  this->OverviewRenderer->GetActiveCamera()->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCamera()
{
  State->updateCamera(RenderView->GetRenderer()->GetActiveCamera());
  State->updateCamera(OverviewRenderer->GetActiveCamera());
  vtkPVRenderView::ResetCamera();
  OverviewRenderer->ResetCamera ( this->LastComputedBounds );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCamera ( double bounds[6] )
{
  State->updateCamera ( RenderView->GetRenderer()->GetActiveCamera() );
  State->updateCamera ( OverviewRenderer->GetActiveCamera() );
  vtkPVRenderView::ResetCamera ( bounds );
  OverviewRenderer->ResetCamera ( bounds );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::ResetCameraClippingRange()
{
  vtkPVRenderView::ResetCameraClippingRange();
  OverviewRenderer->ResetCameraClippingRange ( this->LastComputedBounds );
//   SetCenter ( Center );

}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetOrientationAxesVisibility ( bool )
{
  vtkPVRenderView::SetOrientationAxesVisibility ( false );
}


//----------------------------------------------------------------------------
void vtkPVSliceView::SetBackground ( double r, double g, double b )
{
  vtkPVRenderView::SetBackground ( r,g,b );
  OverviewRenderer->SetBackground ( r,g,b );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetSlice ( double pos )
{
  // qDebug() << "vtkPVSliceView " << SlicingPlane << "changing slice" << pos;
  State->setSlicePosition(SlicingMatrix, pos);
  SliceActor *seg;
  int lowerBound = SlicingPlane * 2;
  int upperBound = SlicingPlane * 2 + 1;
  foreach(seg, Segmentations)
  {
    bool hide = seg->bounds[upperBound] < Center[SlicingPlane] ||
    seg->bounds[lowerBound] > Center[SlicingPlane];
    seg->prop->SetVisibility(seg->visible && !hide && ShowSegmentations);
  }
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetSlicingPlane ( int plane )
{
  if ( SlicingPlane == plane )
    return;

  SlicingPlane = static_cast<VIEW_PLANE> ( plane );

  switch ( SlicingPlane )
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

  State->updateSlicingMatrix ( SlicingMatrix );
  State->updateCamera(RenderView->GetRenderer()->GetActiveCamera());
  State->updateCamera(OverviewRenderer->GetActiveCamera());

  SliceActor *channel;
  foreach(channel, Channels )
  {
    State->updateActor(channel->prop);
  }
  SliceActor *seg;
  foreach(seg, Segmentations )
  {
    State->updateActor(seg->prop);
  }
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetCenter ( double x, double y, double z )
{
//   qDebug() << "vtkPVSliceView setting Center on " << SlicingPlane << x << y << z;
  bool crossHairChanged;

  switch (SlicingPlane)
  {
    case AXIAL:
      crossHairChanged = Center[0] != x || Center[1] != y;
      break;
    case SAGITTAL:
      crossHairChanged = Center[1] != y || Center[2] != z;
      break;
    case CORONAL:
      crossHairChanged = Center[0] != x || Center[2] != z;
  }

  Center[0] = x;
  Center[1] = y;
  Center[2] = z;

  SetSlice ( Center[SlicingPlane] );
  State->setCrossHairs(HCrossLineData,VCrossLineData, Center, LastComputedBounds);

  // Only center camera if center is out of the display view
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(GetRenderer());
  coords->SetCoordinateSystemToNormalizedViewport();
  double ll[3], ur[3];
  coords->SetValue(0, 0); //LL
  memcpy(ll,coords->GetComputedWorldValue(GetRenderer()),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(ur,coords->GetComputedWorldValue(GetRenderer()),3*sizeof(double));

  int H = (SAGITTAL == SlicingPlane)?2:0;
  int V = (CORONAL  == SlicingPlane)?2:1;
  bool centerOutOfCamera = Center[H] < ll[H] || Center[H] > ur[H] // Horizontally out
                   || Center[V] > ll[V] || Center[V] < ur[V];// Vertically out

  if (crossHairChanged && centerOutOfCamera)
  {
    State->updateCamera(RenderView->GetRenderer()->GetActiveCamera(), Center);
//     State->updateCamera(OverviewRenderer->GetActiveCamera(), Center);
  }
}


//----------------------------------------------------------------------------
void vtkPVSliceView::SetCenter(double center[3])
{
  //   qDebug() << "Setting Center3" << center[0] << center[1] << center[2];
  SetCenter (center[0], center[1], center[2]);
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetHCrossLineColor ( double r, double g, double b )
{
  HCrossLine->GetProperty()->SetColor ( r,g,b );
  HCrossLineColor[0] = r;
  HCrossLineColor[1] = g;
  HCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetHCrossLineColor ( double color[3] )
{
  SetHCrossLineColor ( color[0], color[1], color[2] );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetVCrossLineColor ( double r, double g, double b )
{
  VCrossLine->GetProperty()->SetColor ( r,g,b );
  VCrossLineColor[0] = r;
  VCrossLineColor[1] = g;
  VCrossLineColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetVCrossLineColor ( double color[3] )
{
  SetVCrossLineColor ( color[0], color[1], color[2] );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetShowSegmentations ( bool visible )
{
  //   qDebug() << "vtkPVSliceView segmentation's visibility = " << value;
  SliceActor *seg;
  foreach (seg, Segmentations)
  {
    seg->prop->SetVisibility(visible && seg->visible);
  }
  ShowSegmentations = visible;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetShowRuler ( bool visible )
{
  //   qDebug() << "vtkPVSliceView segmentation's visibility = " << value;
  Ruler->SetVisibility ( visible );
  ShowRuler = visible;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetRulerColor ( double r, double g, double b )
{
  Ruler->GetProperty()->SetColor ( r,g,b );
  Ruler->GetLabelTextProperty()->SetColor ( r,g,b );
  Ruler->GetTitleTextProperty()->SetColor ( r,g,b );
  RulerColor[0] = r;
  RulerColor[1] = g;
  RulerColor[2] = b;
}

//----------------------------------------------------------------------------
void vtkPVSliceView::SetRulerColor ( double color[3] )
{
  SetRulerColor ( color[0], color[1], color[2] );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::updateRuler()
{
  if ( !ShowRuler )
    return;

  double *value;
  double left, right;

  double wPad = 60, hPad  = 100;

  int *ws = RenderView->GetRenderWindow()->GetSize();

  bool canDisplayRuler = ws[0] > RulerSize[0] + 2*wPad;
  Ruler->SetVisibility ( canDisplayRuler );
  if ( !canDisplayRuler )
    return;

  Ruler->SetPoint1(wPad/ws[0], hPad/ws[1]);
  Ruler->SetPoint2((wPad+RulerSize[0])/ws[0], hPad/ws[1]);

  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetViewport(GetRenderer());
  coords->SetCoordinateSystemToNormalizedViewport();

  int c = SlicingPlane==SAGITTAL?2:0;
  coords->SetValue(0, 0); //Viewport Lower Left Corner
  value = coords->GetComputedWorldValue ( GetRenderer() );
  left = value[c];
//   qDebug() << "LL" << value[0] << value[1] << value[2];
  coords->SetValue(1, 0); // Viewport Lower Right Corner
  value = coords->GetComputedWorldValue ( GetRenderer() );
  right = value[c];
//   qDebug() << "LR" << value[0] << value[1] << value[2];

  const double RULERWIDTHRATIO = fabs(Ruler->GetPoint1()[0] - Ruler->GetPoint2()[0]);
  double maxRange = fabs ( right-left ) *RULERWIDTHRATIO;
  //       qDebug() << "Max Range" << maxRange;

  std::string units[4] = {"nm","um", "mm", "m"};
  int unit = 0;
  while ( maxRange > 1000 )
  {
    maxRange /= 1000;
    unit++;
  }

  if ( maxRange < 0 && 0 == unit )
    maxRange = 0;
  Ruler->SetRange ( 0, maxRange );
  Ruler->SetTitle ( units[unit].c_str() );
}

//----------------------------------------------------------------------------
void vtkPVSliceView::updateBorder(vtkSmartPointer< vtkPolyData > data,
				  double left, double right,
				  double upper, double lower)
{
  vtkPoints *corners = data->GetPoints();

  switch (SlicingPlane)
  {
    case AXIAL:
      corners->SetPoint(0, left,  upper, 0); //UL
      corners->SetPoint(1, right, upper, 0); //UR
      corners->SetPoint(2, right, lower, 0); //LR
      corners->SetPoint(3, left,  lower, 0); //LL
      break;
    case SAGITTAL:
      corners->SetPoint(0, 0, upper,  left); //UL
      corners->SetPoint(1, 0, lower,  left); //UR
      corners->SetPoint(2, 0, lower, right); //LR
      corners->SetPoint(3, 0, upper, right); //LL
      break;
    case CORONAL:
      corners->SetPoint(0, left,  0, upper); //UL
      corners->SetPoint(1, right, 0, upper); //UR
      corners->SetPoint(2, right, 0, lower); //LR
      corners->SetPoint(3, left,  0, lower); //LL
      break;
    default:
      Q_ASSERT(false);
  }
  data->Modified();
}

//----------------------------------------------------------------------------
void vtkPVSliceView::updateThumbnail()
{
  double *value;
  // Position of world margins acording to the display
  // Depending on the plane being shown can refer to different
  // bound components
  double viewLeft, viewRight, viewUpper, viewLower;
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetViewport(GetRenderer());
  coords->SetCoordinateSystemToNormalizedViewport();

  int h = SlicingPlane==SAGITTAL?2:0;
  int v = SlicingPlane==CORONAL?2:1;
  coords->SetValue(0, 0); // Viewport Lower Left Corner
  value = coords->GetComputedWorldValue ( GetRenderer() );
  viewLower = value[v]; // Lower Margin in World Coordinates
  viewLeft  = value[h]; // Left Margin in World Coordinates
//   qDebug() << "LL" << value[0] << value[1] << value[2];
  coords->SetValue(1, 1);
  value = coords->GetComputedWorldValue(GetRenderer());
  viewUpper = value[v]; // Upper Margin in World Coordinates
  viewRight = value[h]; // Right Margin in Worl Coordinates
//   qDebug() << "UR" << value[0] << value[1] << value[2];

  double sceneLeft  = LastComputedBounds[2*h];
  double sceneRight = LastComputedBounds[2*h+1];
  double sceneUpper = LastComputedBounds[2*v];
  double sceneLower = LastComputedBounds[2*v+1];

  bool leftHidden   = sceneLeft  < viewLeft;
  bool rightHidden  = sceneRight > viewRight;
  bool upperHidden  = sceneUpper < viewUpper;
  bool lowerHidden  = sceneLower > viewLower;

  if (leftHidden || rightHidden || upperHidden || lowerHidden)
  {
    OverviewRenderer->DrawOn();
    updateBorder(SliceBorderData, sceneLeft, sceneRight, sceneUpper, sceneLower);
    updateBorder(ViewBorderData, viewLeft, viewRight, viewUpper, viewLower);
  }
  else
    OverviewRenderer->DrawOff();
}